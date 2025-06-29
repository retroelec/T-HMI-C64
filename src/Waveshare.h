/*
 Copyright (C) 2024-2025 retroelec <retroelec42@gmail.com>

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
#include "OSUtils.h"

static const uint8_t SD_D3_PIN = 21;
static const gpio_num_t PWR_KEY_INPUT_PIN = GPIO_NUM_6;
static const gpio_num_t PWR_CONTROL_PIN = GPIO_NUM_7;

class Waveshare : public BoardDriver {
private:
  OSUtils osUtils;

public:
  void init() override {
    // SD MMC
    vTaskDelay(pdMS_TO_TICKS(500));
    pinMode(SD_D3_PIN, OUTPUT);
    digitalWrite(SD_D3_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(10));

    // calibrate battery
    osUtils.calibrateBattery();

    // activate display
    gpio_reset_pin(PWR_CONTROL_PIN);
    gpio_set_direction(PWR_CONTROL_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(PWR_CONTROL_PIN, 1);
  }

  uint16_t getBatteryVoltage() override {
    // get battery voltage
    int voltage = 0;
    if (osUtils.adc_handle) {
      int raw_value = 0;
      adc_oneshot_read(osUtils.adc_handle, Config::BAT_ADC, &raw_value);
      if (osUtils.adc_cali_handle) {
        if (adc_cali_raw_to_voltage(osUtils.adc_cali_handle, raw_value,
                                    &voltage) != ESP_OK) {
          voltage = raw_value; // fallback
        }
      } else {
        // fallback if calibration was not successful
        voltage = raw_value;
      }
      voltage *= 3;
    }
    return (uint16_t)voltage;
  }

  void powerOff() override {
    // switch off display
    gpio_set_direction(PWR_CONTROL_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(PWR_CONTROL_PIN, 0);

    // configure wakeup (wake on button press)
    esp_sleep_enable_ext0_wakeup(PWR_KEY_INPUT_PIN, 0);

    // deep sleep
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_deep_sleep_start();
  }
};

#endif // WAVESHARE_H
