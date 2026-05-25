/*
 Copyright (C) 2024-2026 retroelec <retroelec42@gmail.com>

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
#ifdef USE_OTA
#include "OtaManager.h"

#include "WiFiManager.h"
#include "platform/PlatformManager.h"

static const char *TAG = "OTA";
static bool otaReady = false;

void OtaManager::start() {
  ArduinoOTA.setHostname("thmi-c64");
  ArduinoOTA.setPassword(NULL);
  ArduinoOTA.onStart([]() {
    PlatformManager::getInstance().log(LOG_INFO, "OTA", "OTA start");
  });
  ArduinoOTA.onEnd(
      []() { PlatformManager::getInstance().log(LOG_INFO, "OTA", "OTA end"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    PlatformManager::getInstance().log(LOG_INFO, "OTA", "Progress: %u%%",
                                       (progress * 100) / total);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    PlatformManager::getInstance().log(LOG_ERROR, "OTA", "Error[%u]: ", error);
  });

  ArduinoOTA.begin();
  otaReady = true;
  PlatformManager::getInstance().log(LOG_INFO, TAG, "OTA ready");
}

void OtaManager::init() {
  PlatformManager::getInstance().log(LOG_INFO, TAG,
                                     "Registering OTA callback to WiFiManager");

  // Register OTA start callback to WiFiManager singleton
  WiFiManager::getInstance()->addConnectedCallback(
      []() { OtaManager::start(); });

  // If WiFi is already connected, start OTA immediately
  if (WiFiManager::getInstance()->isConnected()) {
    OtaManager::start();
  }
}

void OtaManager::handle() {
  if (otaReady) {
    ArduinoOTA.handle();
  }
}

#endif // USE_OTA
#endif // ESP_PLATFORM
