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
#ifndef WAVESHARE_H
#define WAVESHARE_H

#include "BoardDriver.h"
#define SD_D3_PIN 21

class Waveshare : public BoardDriver {
public:
  void init() override {
    // SD MMC
    vTaskDelay(pdMS_TO_TICKS(500));
    pinMode(SD_D3_PIN, OUTPUT);
    digitalWrite(SD_D3_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  void powerOff() override {}

  adc_oneshot_unit_handle_t getAdcHandle() override { return nullptr; }

  adc_cali_handle_t getAdcCaliHandle() override { return nullptr; }
};

#endif // WAVESHARE_H
