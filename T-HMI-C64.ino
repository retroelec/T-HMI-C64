/*
 Copyright (C) 2024 retroelec <retroelec42@gmail.com>

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

#undef CONFIG_LOG_MAXIMUM_LEVEL
#define CONFIG_LOG_MAXIMUM_LEVEL 5

#include "src/C64Emu.h"
#include "src/platform/PlatformManager.h"

static const char *TAG = "T-HMI-C64";

TaskHandle_t core0TaskHandle;

C64Emu c64Emu;

void core0Task(void *param) {
  try {
    c64Emu.setup();
  } catch (...) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "setup() failed");
    while (true) {
    }
  }
  vTaskDelay(pdMS_TO_TICKS(1000));
  PlatformManager::getInstance().log(
      LOG_INFO, TAG, "setup() running on core %d", xPortGetCoreID());
  PlatformManager::getInstance().log(LOG_INFO, TAG, "setup done");

  while (true) {
    c64Emu.loop();
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(500));
  xTaskCreatePinnedToCore(core0Task, "core0Task", 8192, nullptr, 1,
                          &core0TaskHandle, 0);
}

void loop() {}
