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
#include "SDLJoystick.h"
#include "../Config.h"
#ifdef USE_SDLJOYSTICK
#include <SDL2/SDL.h>
#include <cstdint>

uint8_t SDLJoystick::getValue() {
  const uint8_t *state = SDL_GetKeyboardState(NULL);
  uint8_t value = 0xff;
  if (state[SDL_SCANCODE_UP]) {
    value &= ~(1 << 0);
  }
  if (state[SDL_SCANCODE_DOWN]) {
    value &= ~(1 << 1);
  }
  if (state[SDL_SCANCODE_LEFT]) {
    value &= ~(1 << 2);
  }
  if (state[SDL_SCANCODE_RIGHT]) {
    value &= ~(1 << 3);
  }
  if (state[SDL_SCANCODE_LCTRL]) {
    value &= ~(1 << 4);
  }
  return value;
}

bool SDLJoystick::getFire2() {
  const uint8_t *state = SDL_GetKeyboardState(NULL);
  return state[SDL_SCANCODE_RCTRL];
}
#endif
