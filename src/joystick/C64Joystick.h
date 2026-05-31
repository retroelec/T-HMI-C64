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
#ifndef C64JOYSTICK_H
#define C64JOYSTICK_H

#include "../Config.h"
#ifdef USE_C64JOYSTICK
#include "JoystickDriver.h"
#include <cstdint>

class C64Joystick : public JoystickDriver {
private:
  static const uint8_t C64JOYUP = 0;
  static const uint8_t C64JOYDOWN = 1;
  static const uint8_t C64JOYLEFT = 2;
  static const uint8_t C64JOYRIGHT = 3;
  static const uint8_t C64JOYFIRE = 4;

  void init() override;
  uint8_t getValue() override;
  bool getFire2() override;
  bool getJoyOnlyModeButton() override;
};
#endif

#endif // C64JOYSTICK_H
