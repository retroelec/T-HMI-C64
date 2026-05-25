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
#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#ifdef ESP_PLATFORM
#ifdef USE_WIFI

#include <Arduino.h>
#include <ESPAsyncDNSServer.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>
#include <functional>

#define WIFI_MANAGER_AP_SSID "T-HMI-C64"
#define WIFI_MANAGER_AP_PASSWORD ""

inline const char *WIFI_MANAGER_PORTAL_HTML = R"rawliteral(
	<!DOCTYPE html>
	<html>
	<head>
		<meta name="viewport" content="width=device-width, initial-scale=1">
		<meta charset="UTF-8">
		<title>C64 WLAN SETUP</title>
		<style>
			body { font-family: sans-serif; background:#f2f2f2; padding:20px; }
			h2 { text-align:center; }
			select, input, button {
			  box-sizing: border-box;	
			  width:100%; padding:12px; margin:10px 0;
			  border-radius:6px; border:1px solid #ccc;
			}
			button { background:#007bff; color:white; font-size:16px; }
		</style>
		</head>
		<body>
		<h2>SELECT WLAN:</h2>
		<form action="/save" method="POST" accept-charset="UTF-8">
			<select id="networks">%NETWORKS%</select>
			<input type="text" id="ssid" name="ssid" placeholder="NETWORK (SSID)" required>
			<input type="password" name="password" placeholder="PASSWORD">
			<button type="submit">CONNECT</button>
		</form>
		</body>
		<script>
			document.getElementById('networks').addEventListener('change', function () {
    		if (this.value) {
        		document.getElementById('ssid').value = this.value;
    		}
		});
		</script>
	</html>
)rawliteral";

class WiFiManager {
public:
  static WiFiManager *getInstance() {
    static WiFiManager instance;
    return &instance;
  }

  void init(std::function<void()> onConnected = nullptr);
  bool isConnected() const;
  bool isInitialized() const { return initialized; }
  String getIPAddress() const;
  AsyncWebServer *getServer() const;

  // Set RAM pointer for direct memory access (for C64 program uploads)
  void setRamPointer(uint8_t *ramPtr) { ram = ramPtr; }

  // Add callback for when WiFi connects (multiple callbacks supported)
  void addConnectedCallback(std::function<void()> callback);

private:
  WiFiManager(uint16_t port = 80);
  ~WiFiManager();

  // Prevent copying
  WiFiManager(const WiFiManager &) = delete;
  WiFiManager &operator=(const WiFiManager &) = delete;

  void startCaptivePortal();
  void connectToWiFi(const String &ssid, const String &pass);
  String getNetworksHTML();
  void executeConnectedCallbacks();

  uint16_t port;
  AsyncWebServer *server;
  AsyncDNSServer dnsServer;
  Preferences prefs;
  std::vector<std::function<void()>> connectedCallbacks;
  bool connected;
  bool initialized = false;
  bool serverStarted = false;
  uint8_t *ram = nullptr;
};

#endif // USE_WIFI
#endif // ESP_PLATFORM

#endif // WIFIMANAGER_H
