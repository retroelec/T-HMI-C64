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
#ifndef SDLJOYSTICK_H
#define SDLJOYSTICK_H

#include "../Config.h"
#ifdef USE_SDLJOYSTICK
#include "JoystickDriver.h"
#include <cstdint>

struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;

class SDLJoystick : public JoystickDriver {
private:
  SDL_Joystick *m_joystick;
  const int m_deadzone = 16000;

public:
  SDLJoystick();
  virtual ~SDLJoystick();

  void init() override;
  uint8_t getValue() override;
  bool getFire2() override;
  bool getJoyOnlyModeButton() override;
};
#endif

#endif // SDLJOYSTICK_H
