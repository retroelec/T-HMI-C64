/*
  Copyright (C) 2024-2026 retroelec <retroelec42@gmail.com>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 3 of the License, or (at your
  option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  For the complete text of the GNU General Public License see
  http://www.gnu.org/licenses/.
*/
#ifndef SDLKEYBOARDLAYOUT_H
#define SDLKEYBOARDLAYOUT_H

#include "../Config.h"
#ifdef USE_SDL_KEYBOARD

#include "LinuxBaseKeyboardLayout.h"
#include <SDL2/SDL.h>

class SDLKeyboardLayout : public LinuxBaseKeyboardLayout {
protected:
  void addCommonKeys() {
    // generic keys
    LinuxBaseKeyboardLayout::addCommonKeys();

    // SDL-specific special keys
    mapping[{(uint32_t)SDLK_RETURN, false, false}] = C64_KEYCODE_RETURN;
    mapping[{(uint32_t)SDLK_BACKSPACE, false, false}] = C64_KEYCODE_BACKSPACE;
    mapping[{(uint32_t)SDLK_ESCAPE, false, false}] = C64_KEYCODE_BREAK;
    mapping[{(uint32_t)SDLK_UP, false, false}] = C64_KEYCODE_UP;
    mapping[{(uint32_t)SDLK_DOWN, false, false}] = C64_KEYCODE_DOWN;
    mapping[{(uint32_t)SDLK_LEFT, false, false}] = C64_KEYCODE_LEFT;
    mapping[{(uint32_t)SDLK_RIGHT, false, false}] = C64_KEYCODE_RIGHT;
    mapping[{(uint32_t)SDLK_F1, false, false}] = C64_KEYCODE_F1;
    mapping[{(uint32_t)SDLK_F3, false, false}] = C64_KEYCODE_F3;
    mapping[{(uint32_t)SDLK_F5, false, false}] = C64_KEYCODE_F5;
    mapping[{(uint32_t)SDLK_F7, false, false}] = C64_KEYCODE_F7;
    mapping[{(uint32_t)SDLK_HOME, false, false}] = C64_KEYCODE_HOME;
    mapping[{(uint32_t)SDLK_DELETE, false, false}] = C64_KEYCODE_DEL;
    mapping[{(uint32_t)SDLK_HOME, true, false}] = C64_KEYCODE_CLR;
    mapping[{(uint32_t)SDLK_DELETE, true, false}] = C64_KEYCODE_INS;

    mapping[{(uint32_t)SDLK_LSHIFT, false, false}] = C64_KEYCODE_SHIFTLEFT;
    mapping[{(uint32_t)SDLK_RSHIFT, false, false}] = C64_KEYCODE_SHIFTRIGHT;
    mapping[{(uint32_t)SDLK_LALT, false, false}] = C64_KEYCODE_COMMODORE;
    mapping[{(uint32_t)SDLK_LCTRL, false, false}] = C64_KEYCODE_CTRL;
  }

public:
  virtual ~SDLKeyboardLayout() {}
};

#endif
#endif
