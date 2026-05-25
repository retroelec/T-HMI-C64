/*
 Copyright (C) 2025  <uliuc@gmx.net >

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
#include "Config.h"

#ifdef ESP_PLATFORM
#ifdef USE_WIFI
#include "WiFiManager.h"

#include "platform/PlatformManager.h"
#include <algorithm>
#include <cstdlib>
#include <set>
#include <vector>

static const char *TAG = "WIFI";

struct TimerContext {
  std::function<void()> timerFunction;
  esp_timer_handle_t handle;
};

void IRAM_ATTR wifiTimerCallback(void *arg) {
  TimerContext *ctx = static_cast<TimerContext *>(arg);
  if (ctx->timerFunction) {
    ctx->timerFunction();
  }
  esp_timer_stop(ctx->handle);
  esp_timer_delete(ctx->handle);
  delete ctx;
}

static void startOneShotTimer(std::function<void()> timerFunction,
                              uint64_t delay_ms) {
  auto *ctx = new TimerContext{timerFunction, nullptr};
  esp_timer_create_args_t timerArgs = {.callback = &wifiTimerCallback,
                                       .arg = ctx,
                                       .dispatch_method = ESP_TIMER_TASK,
                                       .name = "WiFiOneShot",
                                       .skip_unhandled_events = false};
  esp_timer_handle_t handle;
  ESP_ERROR_CHECK(esp_timer_create(&timerArgs, &handle));
  ctx->handle = handle;
  uint64_t delay_us = delay_ms * 1000;
  ESP_ERROR_CHECK(esp_timer_start_once(handle, delay_us));
}

WiFiManager::WiFiManager(uint16_t port)
    : port(port), server(nullptr), connected(false) {}

WiFiManager::~WiFiManager() {
  if (server) {
    delete server;
  }
}

void WiFiManager::init(std::function<void()> onConnected) {
  if (onConnected) {
    addConnectedCallback(onConnected);
  }

  if (initialized) {
    // If already initialized and connected, execute callbacks immediately
    if (connected) {
      executeConnectedCallbacks();
    }
    return;
  }

  initialized = true;

  PlatformManager::getInstance().log(LOG_INFO, TAG, "Init Wifi");

  // create webserver instance
  server = new AsyncWebServer(port);

// Add C64 program upload endpoint if WIFI_UPLOAD is enabled
#ifdef USE_WIFI_UPLOAD
  server->on(
      "/upload_c64_program", HTTP_POST, [this](AsyncWebServerRequest *request) {
        PlatformManager::getInstance().log(
            LOG_INFO, TAG, "C64 program upload request received");

        // Check for start address parameter
        if (request->hasParam("start", true)) {
          String startParam = request->getParam("start", true)->value();
          uint16_t startAddress = strtoul(startParam.c_str(), nullptr, 16);

          // Check for data
          if (request->hasParam("data", true)) {
            String dataParam = request->getParam("data", true)->value();

            // Convert hex string to binary data
            std::vector<uint8_t> binaryData;
            for (size_t i = 0; i < dataParam.length(); i += 2) {
              if (i + 1 < dataParam.length()) {
                char hexByte[3] = {dataParam[i], dataParam[i + 1], '\0'};
                uint8_t byte = strtoul(hexByte, nullptr, 16);
                binaryData.push_back(byte);
              }
            }

            if (!binaryData.empty() && ram) {
              // Write directly to RAM
              PlatformManager::getInstance().log(
                  LOG_INFO, TAG, "Writing %d bytes to RAM at 0x%x",
                  binaryData.size(), startAddress);

              for (size_t i = 0; i < binaryData.size(); i++) {
                ram[startAddress + i] = binaryData[i];
              }

              request->send(200, "text/plain", "OK: Program loaded to RAM");
              PlatformManager::getInstance().log(
                  LOG_INFO, TAG, "C64 program loaded to RAM: %d bytes at 0x%x",
                  binaryData.size(), startAddress);
            } else {
              request->send(400, "text/plain",
                            "ERROR: No data or RAM pointer not initialized");
            }
          } else {
            request->send(400, "text/plain", "ERROR: Missing 'data' parameter");
          }
        } else {
          request->send(400, "text/plain", "ERROR: Missing 'start' parameter");
        }
      });
#endif // USE_WIFI_UPLOAD

  // event callback to start services when WiFi is ready
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
      PlatformManager::getInstance().log(LOG_INFO, TAG,
                                         "Wifi connected. IP address: %s",
                                         WiFi.localIP().toString());
      connected = true;

      // Start web server for OTA and upload functionality
      // Only start if not already started (e.g., by captive portal)
      if (server && !serverStarted) {
        server->begin();
        serverStarted = true;
      }

      executeConnectedCallbacks();
    }
  });

  prefs.begin("wifi", true);
  String storedSSID = prefs.getString("ssid", "");
  String storedPASS = prefs.getString("pass", "");
  prefs.end();

  if (storedSSID.length()) {
    connectToWiFi(storedSSID, storedPASS);
  } else {
    startCaptivePortal();
  }
}

bool WiFiManager::isConnected() const { return connected; }

String WiFiManager::getIPAddress() const { return WiFi.localIP().toString(); }

AsyncWebServer *WiFiManager::getServer() const { return server; }

String WiFiManager::getNetworksHTML() {
  int n = WiFi.scanNetworks();
  String options;
  std::set<String> seenSSIDs;

  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);

    if (ssid.length() == 0)
      continue;
    if (seenSSIDs.find(ssid) != seenSSIDs.end())
      continue;

    seenSSIDs.insert(ssid);
    options += "<option value='" + ssid + "'>" + ssid + "</option>";
  }

  return options;
}

void WiFiManager::startCaptivePortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_MANAGER_AP_SSID, WIFI_MANAGER_AP_PASSWORD);

  PlatformManager::getInstance().log(LOG_INFO, TAG,
                                     "Wifi access point ip address: %s",
                                     WiFi.softAPIP().toString());

  dnsServer.start(53, "*", WiFi.softAPIP());

  server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
    String page = WIFI_MANAGER_PORTAL_HTML;
    page.replace("%NETWORKS%", this->getNetworksHTML());
    request->send(200, "text/html", page);
  });

  // Android / iOS captive portal triggers
  server->on("/generate_204", HTTP_GET,
             [](AsyncWebServerRequest *r) { r->redirect("/"); });
  server->on("/hotspot-detect.html", HTTP_GET,
             [](AsyncWebServerRequest *r) { r->redirect("/"); });
  server->on("/connecttest.txt", HTTP_GET,
             [](AsyncWebServerRequest *r) { r->redirect("/"); });

  server->on("/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
    String new_ssid = request->getParam("ssid", true)->value();
    String new_pass = request->getParam("password", true)->value();
    request->send(200, "text/html", "<h3>REBOOTING AND CONNECTING...</h3>");

    // save wifi data
    prefs.begin("wifi", false);
    prefs.putString("ssid", new_ssid);
    prefs.putString("pass", new_pass);
    prefs.end();
    delay(1000);

    // reboot
    startOneShotTimer([]() { ESP.restart(); }, 2000);
  });

  // Start web server for captive portal
  if (server) {
    server->begin();
    serverStarted = true;
  }
}
void WiFiManager::connectToWiFi(const String &ssid, const String &pass) {
  PlatformManager::getInstance().log(
      LOG_INFO, TAG, "Trying to connect to ssid %s %s", ssid.c_str());

  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.begin(ssid.c_str(), pass.c_str());

  int attempts = 0;
  const int maxAttempts = 60; // 30 seconds
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    PlatformManager::getInstance().log(
        LOG_INFO, TAG, "Connection to ssid %s established.", ssid);
    connected = true;
    executeConnectedCallbacks();
  } else {
    PlatformManager::getInstance().log(LOG_INFO, TAG,
                                       "Connection to ssid %s failed.", ssid);
    WiFi.mode(WIFI_OFF);
    delay(1000);
    startCaptivePortal();
  }
}

void WiFiManager::executeConnectedCallbacks() {
  PlatformManager::getInstance().log(LOG_INFO, TAG,
                                     "Executing %d connected callbacks",
                                     connectedCallbacks.size());
  for (auto &callback : connectedCallbacks) {
    if (callback) {
      callback();
    }
  }
}

void WiFiManager::addConnectedCallback(std::function<void()> callback) {
  if (callback) {
    connectedCallbacks.push_back(callback);
  }
}

#endif // USE_WIFI
#endif // ESP_PLATFORM
