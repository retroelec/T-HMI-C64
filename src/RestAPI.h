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
#ifndef RESTAPI_H
#define RESTAPI_H

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>

class C64Sys; // forward declaration

/**
 * @brief Simple REST API server for external control of the C64 emulator.
 *
 * Listens on localhost:8080 and provides REST endpoints that translate
 * HTTP requests into ExtCmdQueue entries or return emulator state.
 * Runs in its own thread.
 */
class RestAPI {
private:
  C64Sys *cpu;
  int port;
  int serverFd;
  std::atomic<bool> running;
  std::thread serverThread;

  void serverLoop();
  void handleClient(int clientFd);
  bool parseRequest(const std::string &request, std::string &method,
                    std::string &path, std::string &body);
  void sendResponse(int clientFd, int statusCode, const std::string &statusText,
                    const std::string &contentType = "",
                    const std::string &body = "");
  void routeRequest(const std::string &method, const std::string &path,
                    const std::string &body, int clientFd);

public:
  RestAPI(C64Sys *cpu, int port = 8080);
  ~RestAPI();

  void start();
  void stop();
};

#endif // RESTAPI_H