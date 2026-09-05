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
#ifndef T_HMI_H
#define T_HMI_H

#include "../Config.h"
#ifdef BOARD_T_HMI
#include "BoardDriver.h"
#include "CalibrateBattery.h"
#include "Wakeup.h"
#include <driver/gpio.h>
#include <soc/gpio_struct.h>
#include <stdexcept>

class T_HMI : public BoardDriver, CalibrateBattery {
public:
  void init() override {
    gpio_config_t io_conf;
    io_conf.intr_type = (gpio_int_type_t)GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = (1ULL << Config::PWR_ON) | (1ULL << Config::PWR_EN);
    esp_err_t err = gpio_config(&io_conf);
    ESP_ERROR_CHECK(err);
    GPIO.out_w1ts = (1 << Config::PWR_ON);
    GPIO.out_w1ts = (1 << Config::PWR_EN);
    calibrateBattery();
  }

  uint16_t getBatteryVoltage() override {
    // get battery voltage
    int voltage = 0;
    if (adc_handle) {
      int raw_value = 0;
      adc_oneshot_read(adc_handle, Config::BAT_ADC, &raw_value);
      if (adc_cali_handle) {
        adc_cali_raw_to_voltage(adc_cali_handle, raw_value, &voltage);
        voltage *= 2;
      } else {
        // fallback if calibration was not successful
        voltage = raw_value * 2;
      }
    }
    return (uint16_t)voltage;
  }

  void powerOff() override {
    // turn off the display backlight to save power
    gpio_set_level((gpio_num_t)Config::BL, 0);

#if defined(EXT1_WAKEUP)
    // keep the power latch asserted during deep sleep (board stays
    // powered and can be woken via ext1).
    gpio_set_level((gpio_num_t)Config::PWR_ON, 1);
    gpio_set_level((gpio_num_t)Config::PWR_EN, 1);
    gpio_hold_en((gpio_num_t)Config::PWR_ON);
    gpio_hold_en((gpio_num_t)Config::PWR_EN);
    configureExt1Wakeup(Config::WAKEUP_PIN, true);
#else
    // release the power latch (true hard-off); only the hard/Boot button
    // can power the board on again.
    gpio_set_level((gpio_num_t)Config::PWR_ON, 0);
#endif

    // enter deep sleep
    esp_deep_sleep_start();
  }
};
#endif

#endif // T_HMI_H
