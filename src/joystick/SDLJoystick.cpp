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
#include <math.h>

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
    // Calibrate: measure resting position of both axes
    SDL_JoystickUpdate();
    int32_t sumX = 0, sumY = 0;
    int samples = 8;
    for (int i = 0; i < samples; i++) {
      sumX += SDL_JoystickGetAxis(m_joystick, 0);
      sumY += SDL_JoystickGetAxis(m_joystick, 1);
    }
    m_centerX = static_cast<int16_t>(sumX / samples);
    m_centerY = static_cast<int16_t>(sumY / samples);
  }
}

uint8_t SDLJoystick::getValue() {
  if (!m_joystick)
    return 0xff;
  SDL_JoystickUpdate();
  uint8_t state = 0xff;
  int posX = SDL_JoystickGetAxis(m_joystick, 0) - m_centerX;
  int posY = SDL_JoystickGetAxis(m_joystick, 1) - m_centerY;
  uint8_t hat = SDL_JoystickGetHat(m_joystick, 0);
  // analog values
  double magnitude = sqrt((double)posX * posX + (double)posY * posY);
  if (magnitude > m_deadzone) {
    double normX = posX / magnitude;
    double normY = posY / magnitude;
    double threshold = 0.3826;
    if (normY < -threshold) {
      state &= ~(1 << 0); // UP
    } else if (normY > threshold) {
      state &= ~(1 << 1); // DOWN
    }
    if (normX < -threshold) {
      state &= ~(1 << 2); // LEFT
    } else if (normX > threshold) {
      state &= ~(1 << 3); // RIGHT
    }
  }
  // digital D-pad (hat) overrides/complements the analog values
  if (hat & SDL_HAT_UP)
    state &= ~(1 << 0);
  if (hat & SDL_HAT_DOWN)
    state &= ~(1 << 1);
  if (hat & SDL_HAT_LEFT)
    state &= ~(1 << 2);
  if (hat & SDL_HAT_RIGHT)
    state &= ~(1 << 3);
  // button
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
