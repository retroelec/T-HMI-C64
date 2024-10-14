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

#include "src/C64Emu.h"
#include "src/Config.h"
#include <esp_log.h>

static const char *TAG = "T-HMI-C64";

C64Emu c64Emu;

void setup() {
  Serial.begin(115200);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  ESP_LOGI(TAG, "start setup...");
  ESP_LOGI(TAG, "setup() running on core %d", xPortGetCoreID());
  try {
    c64Emu.setup();
  } catch (...) {
    ESP_LOGE(TAG, "setup() failed");
    while (true) {
    }
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  ESP_LOGI(TAG, "setup done");
}

void loop() { c64Emu.loop(); }
