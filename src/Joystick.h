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
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <cstdint>
#include <driver/gpio.h>

class Joystick {
private:
  static const uint16_t UP_THRESHOLD = 500;
  static const uint16_t DOWN_THRESHOLD = 3500;
  static const uint16_t LEFT_THRESHOLD = 500;
  static const uint16_t RIGHT_THRESHOLD = 3500;

public:
  static const uint8_t C64JOYUP = 0;
  static const uint8_t C64JOYDOWN = 1;
  static const uint8_t C64JOYLEFT = 2;
  static const uint8_t C64JOYRIGHT = 3;
  static const uint8_t C64JOYFIRE = 4;

  void init();
  uint8_t getValue(bool port2, uint8_t dc00, uint8_t dc02);
  bool getFire2();
};
#endif // JOYSTICK_H
