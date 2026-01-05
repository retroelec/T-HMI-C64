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
#include "ArduinoJoystick.h"
#include "../Config.h"
#ifdef USE_ARDUINOJOYSTICK
#include "../platform/PlatformManager.h"
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <soc/gpio_struct.h>
#include <stdexcept>

void ArduinoJoystick::init() {
  // init adc (x and y axis)
  adc_oneshot_unit_init_cfg_t init_config = {.unit_id = ADC_UNIT_2,
                                             .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
                                             .ulp_mode = ADC_ULP_MODE_DISABLE};
  esp_err_t err = adc_oneshot_new_unit(&init_config, &adc2_handle);
  if (err != ESP_OK) {
    throw std::runtime_error(esp_err_to_name(err));
  }
  adc_oneshot_chan_cfg_t channel_config = {.atten = ADC_ATTEN_DB_12,
                                           .bitwidth = ADC_BITWIDTH_DEFAULT};
  adc_oneshot_config_channel(adc2_handle, Config::ADC_JOYSTICK_X,
                             &channel_config);
  adc_oneshot_config_channel(adc2_handle, Config::ADC_JOYSTICK_Y,
                             &channel_config);
  // init gpio (fire buttons)
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  io_conf.pin_bit_mask = (1ULL << Config::JOYSTICK_FIRE_PIN) |
                         (1ULL << Config::JOYSTICK_FIRE2_PIN);

  err = gpio_config(&io_conf);
  if (err != ESP_OK) {
    throw std::runtime_error(esp_err_to_name(err));
  }
  // init other attributes
  lastjoystickvalue = 0xff;
  lastMeasuredTime = PlatformManager::getInstance().getTimeUS();
}

uint8_t ArduinoJoystick::getValue() {
  // do not check joystick value too often (max only each 10 ms)
  int64_t actMeasuredTime = PlatformManager::getInstance().getTimeUS();
  if ((actMeasuredTime - lastMeasuredTime) < 10000) {
    return lastjoystickvalue;
  }
  lastMeasuredTime = actMeasuredTime;
  // 2048 = medium adc value (12-bit resolution)
  int valueX = 2048;
  int valueY = 2048;
  uint8_t valueFire;
  adc_oneshot_read(adc2_handle, Config::ADC_JOYSTICK_X, &valueX);
  adc_oneshot_read(adc2_handle, Config::ADC_JOYSTICK_Y, &valueY);
  valueFire = (GPIO.in >> Config::JOYSTICK_FIRE_PIN) & 0x01;
  // C64 register value
  uint8_t value = 0xff;
  if (valueX < LEFT_THRESHOLD) {
    value &= ~(1 << C64JOYLEFT);
  } else if (valueX > RIGHT_THRESHOLD) {
    value &= ~(1 << C64JOYRIGHT);
  }
  if (valueY < UP_THRESHOLD) {
    value &= ~(1 << C64JOYDOWN);
  } else if (valueY > DOWN_THRESHOLD) {
    value &= ~(1 << C64JOYUP);
  }
  if (valueFire == 0) {
    value &= ~(1 << C64JOYFIRE);
  }
  lastjoystickvalue = value;
  return value;
}

bool ArduinoJoystick::getFire2() {
  return ((GPIO.in >> Config::JOYSTICK_FIRE2_PIN) & 0x01) == 0;
}
#endif
