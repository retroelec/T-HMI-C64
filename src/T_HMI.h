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
#ifndef T_HMI_H
#define T_HMI_H

#include "BoardDriver.h"
#include "Config.h"
#include "HardwareInitializationException.h"
#include <driver/gpio.h>
#include <soc/gpio_struct.h>

class T_HMI : public BoardDriver {
public:
  void init() override {
    gpio_config_t io_conf;
    io_conf.intr_type = (gpio_int_type_t)GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = (1ULL << Config::PWR_ON) | (1ULL << Config::PWR_EN);
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
      throw HardwareInitializationException(
          std::string("init. of BoardDriver failed: ") + esp_err_to_name(err));
    }
    GPIO.out_w1ts = (1 << Config::PWR_ON);
    GPIO.out_w1ts = (1 << Config::PWR_EN);
  }
};

#endif // T_HMI_H
