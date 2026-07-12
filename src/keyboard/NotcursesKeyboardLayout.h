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
#ifndef NOTCURSESKEYBOARDLAYOUT_H
#define NOTCURSESKEYBOARDLAYOUT_H

#include "../Config.h"
#ifdef USE_NOTCURSES_KEYBOARD

#include "LinuxBaseKeyboardLayout.h"
#include <notcurses/notcurses.h>

class NotcursesKeyboardLayout : public LinuxBaseKeyboardLayout {
protected:
  void addCommonKeys() {
    LinuxBaseKeyboardLayout::addCommonKeys();

    mapping[{(uint32_t)NCKEY_ENTER, false, false}] = C64_KEYCODE_RETURN;
    mapping[{(uint32_t)NCKEY_BACKSPACE, false, false}] = C64_KEYCODE_BACKSPACE;
    mapping[{0x1b, false, false}] = C64_KEYCODE_BREAK;

    mapping[{(uint32_t)NCKEY_UP, false, false}] = C64_KEYCODE_UP;
    mapping[{(uint32_t)NCKEY_DOWN, false, false}] = C64_KEYCODE_DOWN;
    mapping[{(uint32_t)NCKEY_LEFT, false, false}] = C64_KEYCODE_LEFT;
    mapping[{(uint32_t)NCKEY_RIGHT, false, false}] = C64_KEYCODE_RIGHT;

    mapping[{(uint32_t)NCKEY_F01, false, false}] = C64_KEYCODE_F1;
    mapping[{(uint32_t)NCKEY_F03, false, false}] = C64_KEYCODE_F3;
    mapping[{(uint32_t)NCKEY_F05, false, false}] = C64_KEYCODE_F5;
    mapping[{(uint32_t)NCKEY_F07, false, false}] = C64_KEYCODE_F7;

    mapping[{(uint32_t)NCKEY_HOME, false, false}] = C64_KEYCODE_HOME;
    mapping[{(uint32_t)NCKEY_DEL, false, false}] = C64_KEYCODE_DEL;
    mapping[{(uint32_t)NCKEY_INS, false, false}] = C64_KEYCODE_INS;
  }

public:
  virtual uint32_t normalizeKey(uint32_t ch) const {
    if (ch >= 'A' && ch <= 'Z') {
      return ch + 0x20;
    }
    return ch;
  }

  virtual ~NotcursesKeyboardLayout() {}
};

#endif
#endif