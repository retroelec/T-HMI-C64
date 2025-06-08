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
#include "OSUtils.h"
#include <driver/gpio.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_oneshot.h>
#include <soc/gpio_struct.h>

class T_HMI : public BoardDriver {
private:
  adc_oneshot_unit_handle_t adc1_handle;
  adc_cali_handle_t adc_cali_handle;

  void calibrateBattery() {
    // initialize ADC unit 1 in one-shot mode with default RTC clock source
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE};
    adc_oneshot_new_unit(&init_config, &adc1_handle);
    // configure the selected ADC channel (for battery voltage measurement)
    // - attenuation of 12 dB allows input voltages up to ~3.3V
    // - default bit width (typically 12 bits)
    adc_oneshot_chan_cfg_t config = {.atten = ADC_ATTEN_DB_12,
                                     .bitwidth = ADC_BITWIDTH_DEFAULT};
    adc_oneshot_config_channel(adc1_handle, Config::BAT_ADC, &config);
    // configure ADC calibration using curve fitting scheme
    // this uses calibration data (if available) stored in eFuses
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = Config::BAT_ADC,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    esp_err_t ret =
        adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle);
    // check if calibration was successful; log error if it failed
    if (ret != ESP_OK) {
      OSUtils::log(LOG_ERROR, "T-HMI", "adc calibration failed");
    }
  }

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
    calibrateBattery();
  }

  void powerOff() override {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << Config::PWR_ON);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)Config::PWR_ON, 0);
  }

  adc_oneshot_unit_handle_t getAdcHandle() override { return adc1_handle; }

  adc_cali_handle_t getAdcCaliHandle() override { return adc_cali_handle; }
};

#endif // T_HMI_H
