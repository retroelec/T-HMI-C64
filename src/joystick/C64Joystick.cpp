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
#include "../Config.h"
#ifdef USE_C64JOYSTICK
#include "../platform/PlatformManager.h"
#include "C64Joystick.h"
#include <driver/gpio.h>
#include <soc/gpio_struct.h>

static const char *TAG = "C64Joystick";

void C64Joystick::init() {
  // init gpio (fire buttons)
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  io_conf.pin_bit_mask = (1ULL << Config::JOYSTICK_UP_PIN) |
                         (1ULL << Config::JOYSTICK_DOWN_PIN) |
                         (1ULL << Config::JOYSTICK_LEFT_PIN) |
                         (1ULL << Config::JOYSTICK_RIGHT_PIN) |
                         (1ULL << Config::JOYSTICK_FIRE_PIN) |
                         (1ULL << Config::JOYSTICK_FIRE2_PIN);

  esp_err_t err = gpio_config(&io_conf);
  if (err != ESP_OK) {
    PlatformManager::getInstance().log(
        LOG_INFO, TAG, "error in init. of joystick: %s - continue anyway",
        esp_err_to_name(err));
    return;
  }
}

uint8_t C64Joystick::getValue() {
  uint8_t valueUp = (GPIO.in.val >> Config::JOYSTICK_UP_PIN) & 0x01;
  uint8_t valueDown = (GPIO.in.val >> Config::JOYSTICK_DOWN_PIN) & 0x01;
  uint8_t valueLeft = (GPIO.in.val >> Config::JOYSTICK_LEFT_PIN) & 0x01;
  uint8_t valueRight = (GPIO.in.val >> Config::JOYSTICK_RIGHT_PIN) & 0x01;
  uint8_t valueFire = (GPIO.in.val >> Config::JOYSTICK_FIRE_PIN) & 0x01;
  uint8_t value = 0xff;
  if (valueUp == 0) {
    value &= ~(1 << C64JOYUP);
  }
  if (valueDown == 0) {
    value &= ~(1 << C64JOYDOWN);
  }
  if (valueLeft == 0) {
    value &= ~(1 << C64JOYLEFT);
  }
  if (valueRight == 0) {
    value &= ~(1 << C64JOYRIGHT);
  }
  if (valueFire == 0) {
    value &= ~(1 << C64JOYFIRE);
  }
  return value;
}

bool C64Joystick::getFire2() {
  return ((GPIO.in.val >> Config::JOYSTICK_FIRE2_PIN) & 0x01) == 0;
}

bool C64Joystick::getJoyOnlyModeButton() {
  return ((GPIO.in.val >> Config::JOYSTICK_FIRE2_PIN) & 0x01) == 0;
}
#endif
