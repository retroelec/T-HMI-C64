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
#include "Joystick.h"
#include "Config.h"
#include "JoystickInitializationException.h"
#include <driver/adc.h>

void Joystick::init() {
  // init adc (x and y axis)
  adc2_config_channel_atten(Config::ADC_JOYSTICK_X, ADC_ATTEN_11db);
  adc2_config_channel_atten(Config::ADC_JOYSTICK_Y, ADC_ATTEN_11db);
  // init gpio (fire button)
  gpio_config_t io_conf;
  io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  io_conf.pin_bit_mask = (1ULL << Config::JOYSTICK_FIRE_PIN);
  esp_err_t err = gpio_config(&io_conf);
  if (err != ESP_OK) {
    throw JoystickInitializationException(esp_err_to_name(err));
  }
}

uint8_t Joystick::getValue(bool port2) {
  // assume return value of adc2_get_raw is ESP_OK
  int valueX;
  adc2_get_raw(Config::ADC_JOYSTICK_X, ADC_WIDTH_12Bit, &valueX);
  int valueY;
  adc2_get_raw(Config::ADC_JOYSTICK_Y, ADC_WIDTH_12Bit, &valueY);
  // GPIO.in1.val must be used for GPIO pins > 32
  uint8_t valueFire = (GPIO.in >> Config::JOYSTICK_FIRE_PIN) & 0x01;
  // C64 register value
  uint8_t value = 0xff;
  if (port2) {
    value = 0x7f;
  }
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
  return value;
}
