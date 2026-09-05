/*
  Copyright (C) 2024-2026 retroelec <retroelec42@gmail.com>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 3 of the License, or (at your
  option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  Public License for more details.

  For the complete text of the GNU General Public License see
  http://www.gnu.org/licenses/.
*/
#ifndef WAKEUP_H
#define WAKEUP_H

#include "../Config.h"

#ifdef ESP_PLATFORM
#include <driver/gpio.h>
#include <esp_sleep.h>

/**
  @brief Configures ext1 (external) wakeup from deep sleep on a single GPIO.

  The pin is configured as an input with the appropriate pull resistor so it
  has a defined level during sleep:

    - activeLow  = true : pin is pulled up, wakes when driven LOW
                            (button to GND, ESP_EXT1_WAKEUP_ANY_LOW)
    - activeLow  = false: pin is pulled down, wakes when driven HIGH
                            (ESP_EXT1_WAKEUP_ANY_HIGH)

  @note The wakeup pin MUST be an RTC GPIO (ESP32-S3: GPIO 0-21).
*/
static inline void configureExt1Wakeup(gpio_num_t pin, bool activeLow) {
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = (1ULL << pin);
  if (activeLow) {
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  } else {
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
  }
  gpio_config(&io_conf);

  uint64_t mask = (1ULL << pin);
  esp_sleep_enable_ext1_wakeup(mask, activeLow ? ESP_EXT1_WAKEUP_ANY_LOW
                                               : ESP_EXT1_WAKEUP_ANY_HIGH);
}

#endif // ESP_PLATFORM
#endif // WAKEUP_H
