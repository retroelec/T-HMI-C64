/*
 Copyright (C) 2024-2026 retroelec <retroelec42@gmail.com>
 (original code extensions)

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 3 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 for more details.

 For the complete text of the GNU General Public License see
 http://www.gnu.org/licenses/.
*/
#include "RestAPI.h"

#ifdef PLATFORM_LINUX

#include "C64Sys.h"
#include "Config.h"
#include "ExtCmd.h"
#include "ExtCmdQueue.h"
#include "VIC.h"
#include "nlohmann/json.hpp"
#include "platform/PlatformManager.h"

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

static const char *TAG = "RestAPI";

// C64 16-color palette (R, G, B)
static const uint8_t C64_PALETTE[16][3] = {
    {0, 0, 0},       // 0: black
    {255, 255, 255}, // 1: white
    {136, 0, 0},     // 2: red
    {170, 255, 238}, // 3: cyan
    {0, 0, 136},     // 4: purple
    {136, 0, 136},   // 5: green
    {0, 136, 0},     // 6: blue
    {170, 170, 170}, // 7: yellow
    {170, 170, 170}, // 8: orange
    {0, 170, 255},   // 9: brown
    {255, 136, 0},   // 10: light red
    {136, 170, 0},   // 11: dark gray
    {0, 136, 170},   // 12: medium gray
    {119, 68, 17},   // 13: light green
    {102, 102, 170}, // 14: light blue
    {187, 187, 187}  // 15: light gray
};

RestAPI::RestAPI(C64Sys *cpu, int port)
    : cpu(cpu), port(port), serverFd(-1), running(false) {}

RestAPI::~RestAPI() { stop(); }

void RestAPI::start() {
  if (running.load(std::memory_order_acquire)) {
    PlatformManager::getInstance().log(LOG_INFO, TAG,
                                       "already running on port %d", port);
    return;
  }

  running.store(true, std::memory_order_release);
  serverThread = std::thread(&RestAPI::serverLoop, this);
  serverThread.detach();
  PlatformManager::getInstance().log(
      LOG_INFO, TAG, "started on port %d (localhost only)", port);
}

void RestAPI::stop() {
  running.store(false, std::memory_order_release);
  if (serverFd >= 0) {
    ::shutdown(serverFd, SHUT_RDWR);
    ::close(serverFd);
    serverFd = -1;
  }
  PlatformManager::getInstance().log(LOG_INFO, TAG, "stopped");
}

void RestAPI::serverLoop() {
  serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd < 0) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                       "failed to create socket");
    running.store(false, std::memory_order_release);
    return;
  }

  int opt = 1;
  if (::setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                       "failed to set SO_REUSEADDR");
  }

  struct sockaddr_in address;
  std::memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // localhost only
  address.sin_port = htons(port);

  if (::bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                       "failed to bind to port %d", port);
    ::close(serverFd);
    serverFd = -1;
    running.store(false, std::memory_order_release);
    return;
  }

  if (::listen(serverFd, 5) < 0) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                       "failed to listen on socket");
    ::close(serverFd);
    serverFd = -1;
    running.store(false, std::memory_order_release);
    return;
  }

  PlatformManager::getInstance().log(LOG_INFO, TAG,
                                     "listening on http://localhost:%d", port);

  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);

  while (running.load(std::memory_order_acquire)) {
    int clientFd =
        ::accept(serverFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientFd < 0) {
      if (running.load(std::memory_order_acquire)) {
        PlatformManager::getInstance().log(LOG_ERROR, TAG, "accept() failed");
      }
      break;
    }
    handleClient(clientFd);
    ::close(clientFd);
  }

  ::close(serverFd);
  serverFd = -1;
  PlatformManager::getInstance().log(LOG_INFO, TAG, "server loop ended");
}

void RestAPI::handleClient(int clientFd) {
  // Read request (HTTP headers + body)
  std::string request;
  char buf[4096];
  bool headerEndFound = false;
  int contentLength = 0;

  while (true) {
    ssize_t n = ::read(clientFd, buf, sizeof(buf) - 1);
    if (n <= 0) {
      break;
    }
    buf[n] = '\0';
    request.append(buf, n);

    // Check if we have reached end of headers
    size_t headerEnd = request.find("\r\n\r\n");
    if (!headerEndFound && headerEnd != std::string::npos) {
      headerEndFound = true;
      // Parse Content-Length from headers
      size_t clPos = request.find("Content-Length: ");
      if (clPos != std::string::npos) {
        clPos += 16; // length of "Content-Length: "
        size_t clEnd = request.find("\r\n", clPos);
        std::string clStr = request.substr(clPos, clEnd - clPos);
        contentLength = std::stoi(clStr);
      }
      // If we already have the full body, stop reading
      size_t bodyStart = headerEnd + 4;
      size_t bodyLen = request.size() - bodyStart;
      if (bodyLen >= (size_t)contentLength) {
        break;
      }
    }
  }

  // Parse and route the request
  std::string method, path, body;
  if (parseRequest(request, method, path, body)) {
    routeRequest(method, path, body, clientFd);
  }
}

bool RestAPI::parseRequest(const std::string &request, std::string &method,
                           std::string &path, std::string &body) {
  // Parse request line: "POST /api/load HTTP/1.1"
  size_t firstSpace = request.find(' ');
  if (firstSpace == std::string::npos) {
    return false;
  }
  method = request.substr(0, firstSpace);

  size_t secondSpace = request.find(' ', firstSpace + 1);
  if (secondSpace == std::string::npos) {
    return false;
  }
  path = request.substr(firstSpace + 1, secondSpace - firstSpace - 1);

  // Extract body after headers
  size_t headerEnd = request.find("\r\n\r\n");
  if (headerEnd != std::string::npos) {
    body = request.substr(headerEnd + 4);
  }

  return true;
}

void RestAPI::sendResponse(int clientFd, int statusCode,
                           const std::string &statusText,
                           const std::string &contentType,
                           const std::string &body) {
  std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " +
                         statusText +
                         "\r\n"
                         "Access-Control-Allow-Origin: *\r\n"
                         "Connection: close\r\n";

  if (!body.empty()) {
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
  } else {
    response += "Content-Length: 0\r\n";
  }

  response += "\r\n";
  if (!body.empty()) {
    response += body;
  }

  ::write(clientFd, response.c_str(), response.size());
}

void RestAPI::routeRequest(const std::string &method, const std::string &path,
                           const std::string &body, int clientFd) {
  // PlatformManager::getInstance().log(LOG_INFO, TAG, "%s %s", method.c_str(),
  //                                    path.c_str());

  // Handle CORS preflight
  if (method == "OPTIONS") {
    return; // sendResponse will include CORS headers
  }

  // ----- POST endpoints (commands) -----
  if (method == "POST") {
    std::string filename;
    ExtCmdQueue::ExternalCmd extcmd;
    std::memset(&extcmd, 0, sizeof(extcmd));

    if (path == "/api/load") {
      try {
        auto json = nlohmann::json::parse(body);
        filename = json["filename"].get<std::string>();
      } catch (...) {
        PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                           "invalid JSON body for /api/load");
        sendResponse(clientFd, 400, "Bad Request");
        return;
      }
      // Remove .prg extension if present (AUTOSTART adds it)
      if (filename.size() > 4 &&
          filename.substr(filename.size() - 4) == ".prg") {
        filename = filename.substr(0, filename.size() - 4);
      }
      extcmd.cmd = ExtCmd::AUTOSTART;
      std::copy(filename.begin(), filename.end(), &extcmd.param[2]);
      extcmd.param[2 + filename.size()] = '\0';
      ExtCmdQueue::getInstance().push(extcmd);
      PlatformManager::getInstance().log(
          LOG_INFO, TAG, "/api/load: queued AUTOSTART '%s'", filename.c_str());

    } else if (path == "/api/reset") {
      extcmd.cmd = ExtCmd::RESET;
      ExtCmdQueue::getInstance().push(extcmd);
      PlatformManager::getInstance().log(LOG_INFO, TAG,
                                         "/api/reset: queued RESET");

    } else if (path == "/api/pause") {
      extcmd.cmd = ExtCmd::PAUSE;
      ExtCmdQueue::getInstance().push(extcmd);
      PlatformManager::getInstance().log(LOG_INFO, TAG,
                                         "/api/pause: queued PAUSE");

    } else if (path == "/api/key") {
      try {
        auto json = nlohmann::json::parse(body);
        extcmd.param[0] = json["dc00"].get<int>();
        extcmd.param[1] = json["dc01"].get<int>();
        extcmd.param[2] = json.value("shift", 0);
        extcmd.param[3] = json.value("duration", 3);
      } catch (...) {
        PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                           "invalid JSON body for /api/key");
        return;
      }
      extcmd.cmd = ExtCmd::SENDKEY;
      ExtCmdQueue::getInstance().push(extcmd);
      PlatformManager::getInstance().log(
          LOG_INFO, TAG, "/api/key: queued SENDKEY dc00=%x dc01=%x",
          extcmd.param[0], extcmd.param[1]);

    } else if (path == "/api/joy") {
      try {
        auto json = nlohmann::json::parse(body);
        extcmd.param[0] = json["value"].get<int>();
      } catch (...) {
        PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                           "invalid JSON body for /api/joy");
        return;
      }
      extcmd.cmd = ExtCmd::SENDJOY;
      ExtCmdQueue::getInstance().push(extcmd);
      // PlatformManager::getInstance().log(
      //     LOG_INFO, TAG, "/api/joy: queued SENDJOY value=%x",
      //     extcmd.param[0]);

    } else if (path == "/api/kbjoystick") {
      // POST /api/kbjoystick – set keyboard joystick mode
      try {
        auto json = nlohmann::json::parse(body);
        int mode = json["mode"].get<int>();
        if (mode == 1) {
          extcmd.cmd = ExtCmd::KBJOYSTICKMODE1;
        } else if (mode == 2) {
          extcmd.cmd = ExtCmd::KBJOYSTICKMODE2;
        } else {
          extcmd.cmd = ExtCmd::KBJOYSTICKMODEOFF;
        }
      } catch (...) {
        PlatformManager::getInstance().log(
            LOG_ERROR, TAG, "invalid JSON body for /api/kbjoystick");
        return;
      }
      ExtCmdQueue::getInstance().push(extcmd);
      PlatformManager::getInstance().log(LOG_INFO, TAG,
                                         "/api/kbjoystick: queued mode");

    } else if (path == "/api/savestate") {
      extcmd.cmd = ExtCmd::SAVESNAPSHOT;
      ExtCmdQueue::getInstance().push(extcmd);
      PlatformManager::getInstance().log(LOG_INFO, TAG,
                                         "/api/savestate: queued SAVESNAPSHOT");

    } else if (path == "/api/loadstate") {
      extcmd.cmd = ExtCmd::LOADSNAPSHOT;
      ExtCmdQueue::getInstance().push(extcmd);
      PlatformManager::getInstance().log(LOG_INFO, TAG,
                                         "/api/loadstate: queued LOADSNAPSHOT");

    } else {
      PlatformManager::getInstance().log(LOG_INFO, TAG, "unknown POST path: %s",
                                         path.c_str());
      sendResponse(clientFd, 404, "Not Found");
      return;
    }

    sendResponse(clientFd, 200, "OK");

    // ----- GET endpoints (data queries) -----
  } else if (method == "GET") {
    if (path == "/api/screen/text") {
      // Build JSON with 40x25 character grid + colors
      nlohmann::json j;
      j["cols"] = 40;
      j["rows"] = 25;

      auto &chars = j["chars"] = nlohmann::json::array();
      auto &colors = j["colors"] = nlohmann::json::array();

      for (int row = 0; row < 25; row++) {
        auto charRow = nlohmann::json::array();
        auto colorRow = nlohmann::json::array();
        for (int col = 0; col < 40; col++) {
          int idx = row * 40 + col;
          uint8_t ch = cpu->getMem(0x0400 + idx);
          uint8_t co = cpu->getMem(0xD800 + idx) & 0x0f;
          charRow.push_back(ch);
          colorRow.push_back(co);
        }
        chars.push_back(charRow);
        colors.push_back(colorRow);
      }

      std::string jsonStr = j.dump();
      sendResponse(clientFd, 200, "OK", "application/json", jsonStr);

    } else if (path == "/api/screen/ppm") {
      // Build PPM image (P6 binary format)
      uint8_t *bitmap = cpu->vic.getBitmap();

      std::string ppm;
      ppm.reserve(15 + 320 * 200 * 3); // header + RGB data

      // PPM header
      ppm += "P6\n320 200\n255\n";

      // Pixel data (convert C64 color indices to RGB)
      for (int i = 0; i < 320 * 200; i++) {
        int ci = bitmap[i] & 0x0f;
        ppm += (char)C64_PALETTE[ci][0];
        ppm += (char)C64_PALETTE[ci][1];
        ppm += (char)C64_PALETTE[ci][2];
      }

      sendResponse(clientFd, 200, "OK", "image/x-portmap", ppm);

    } else if (path == "/api/status") {
      // Build JSON with emulator status
      nlohmann::json j;
      j["running"] = !cpu->cpuhalted;
      j["paused"] = cpu->cpuhalted;
      j["joystickMode"] = cpu->joystickmode;
      j["kbJoystickMode"] = cpu->kbjoystickmode;
      j["volume"] = cpu->sid.getEmuVolume();
      j["perf"] = cpu->perf.load(std::memory_order_acquire);
      j["debug"] = cpu->debug;
      j["detectReleaseKey"] = cpu->detectreleasekey;

      std::string jsonStr = j.dump();
      sendResponse(clientFd, 200, "OK", "application/json", jsonStr);

    } else if (path == "/api/registers") {
      // Build JSON with CPU register values
      nlohmann::json j;
      j["pc"] = cpu->getPC();
      j["a"] = cpu->getA();
      j["x"] = cpu->getX();
      j["y"] = cpu->getY();
      j["sr"] = cpu->getSR();
      j["sp"] = cpu->getSP();

      std::string jsonStr = j.dump();
      sendResponse(clientFd, 200, "OK", "application/json", jsonStr);

    } else if (path.substr(0, 12) == "/api/memory?") {
      // GET /api/memory?addr=0x1000&len=32 – read RAM bytes
      std::string q = path.substr(12);
      uint16_t addr = 0;
      uint32_t len = 16;
      size_t ap = q.find("addr=");
      size_t lp = q.find("len=");
      if (ap != std::string::npos) {
        addr = std::stoul(q.substr(ap + 5), nullptr, 0);
      }
      if (lp != std::string::npos) {
        len = std::stoul(q.substr(lp + 4), nullptr, 0);
        if (len > 65535)
          len = 65535;
      }
      nlohmann::json j;
      j["addr"] = addr;
      j["len"] = len;
      auto &data = j["data"] = nlohmann::json::array();
      for (uint16_t i = 0; i < len; i++) {
        data.push_back(cpu->getMem(addr + i));
      }
      std::string jsonStr = j.dump();
      sendResponse(clientFd, 200, "OK", "application/json", jsonStr);

    } else {
      PlatformManager::getInstance().log(LOG_INFO, TAG, "unknown GET path: %s",
                                         path.c_str());
      sendResponse(clientFd, 404, "Not Found");
    }

  } else {
    sendResponse(clientFd, 405, "Method Not Allowed");
  }
}
#endif // PLATFORM_LINUX
