/*
 Copyright (C) 2025 retroelec <retroelec42@gmail.com>

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
#ifndef SDLJOYSTICK_H
#define SDLJOYSTICK_H

#include "../Config.h"
#ifdef USE_SDLJOYSTICK
#include "JoystickDriver.h"
#include <cstdint>

class SDLJoystick : public JoystickDriver {
public:
  uint8_t getValue() override;
  bool getFire2() override;
};
#endif

#endif // SDLJOYSTICK_H
