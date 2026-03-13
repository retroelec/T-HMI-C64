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
#ifdef USE_SDLJOYSTICK
#include "SDLJoystick.h"
#include <SDL2/SDL.h>
#include <cstdint>

SDLJoystick::SDLJoystick() : m_joystick(nullptr) {}

SDLJoystick::~SDLJoystick() {
  if (m_joystick) {
    SDL_JoystickClose(m_joystick);
  }
}

void SDLJoystick::init() {
  if (SDL_WasInit(SDL_INIT_JOYSTICK) == 0) {
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
  }
  if (SDL_NumJoysticks() > 0) {
    m_joystick = SDL_JoystickOpen(0);
  }
}

uint8_t SDLJoystick::getValue() {
  if (!m_joystick)
    return 0xff;
  SDL_JoystickUpdate();
  uint8_t state = 0xff;
  int16_t posX = SDL_JoystickGetAxis(m_joystick, 0);
  int16_t posY = SDL_JoystickGetAxis(m_joystick, 1);
  uint8_t hat = SDL_JoystickGetHat(m_joystick, 0);
  if (posY < -m_deadzone || (hat & SDL_HAT_UP))
    state &= ~(1 << 0);
  if (posY > m_deadzone || (hat & SDL_HAT_DOWN))
    state &= ~(1 << 1);
  if (posX < -m_deadzone || (hat & SDL_HAT_LEFT))
    state &= ~(1 << 2);
  if (posX > m_deadzone || (hat & SDL_HAT_RIGHT))
    state &= ~(1 << 3);
  if (SDL_JoystickGetButton(m_joystick, 0)) {
    state &= ~(1 << 4);
  }
  return state;
}

bool SDLJoystick::getFire2() {
  if (!m_joystick)
    return false;
  SDL_JoystickUpdate();
  return SDL_JoystickGetButton(m_joystick, 1) != 0;
}

bool SDLJoystick::getJoyOnlyModeButton() {
  return SDL_JoystickGetButton(m_joystick, 2) != 0;
}
#endif
