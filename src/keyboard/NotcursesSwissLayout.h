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
#ifndef NOTCURSESSWISSLAYOUT_H
#define NOTCURSESSWISSLAYOUT_H

#include "../Config.h"
#ifdef USE_NOTCURSES_KEYBOARD

#include "NotcursesKeyboardLayout.h"

class NotcursesSwissLayout : public NotcursesKeyboardLayout {
public:
  uint32_t normalizeKey(uint32_t ch) const override {
    if (ch >= 'A' && ch <= 'Z') {
      return ch + 0x20;
    }
    switch (ch) {
    case '!':
      return '1';
    case '@':
      return '2';
    case '#':
      return '3';
    case '$':
      return '4';
    case '%':
      return '5';
    case '^':
      return '6';
    case '&':
      return '7';
    case '*':
      return '8';
    case '(':
      return '9';
    case ')':
      return '0';
    case 0xa7:
      return '3';
    case 0xb0:
      return '0';
    case 0xe8:
    case 0xe9:
      return 'e';
    case 0xfc:
      return 'u';
    case 0xf6:
      return 'o';
    case 0xe4:
      return 'a';
    case 0xe0:
      return 'a';
    case 0xe7:
      return 'c';
    }
    return ch;
  }

  NotcursesSwissLayout() {
    layoutName = "Swiss";
    addCommonKeys();

    mapping[{0x2d, false, false}] = C64_KEYCODE_MINUS;
    mapping[{0x2e, false, false}] = C64_KEYCODE_PERIOD;
    mapping[{0x2c, false, false}] = C64_KEYCODE_COMMA;
    mapping[{0x24, false, false}] = C64_KEYCODE_DOLLAR;
    mapping[{0x3c, false, false}] = C64_KEYCODE_LESS;
    mapping[{0x27, false, false}] = C64_KEYCODE_QUOTE;

    mapping[{0x27, false, false}] = C64_KEYCODE_QUESTION;
    mapping[{0x2b, false, false}] = C64_KEYCODE_PLUS;
    mapping[{0x2a, false, false}] = C64_KEYCODE_ASTERISK;
    mapping[{0x37, false, false}] = C64_KEYCODE_SLASH;
    mapping[{0x3d, false, false}] = C64_KEYCODE_EQUALS;
    mapping[{0x3a, false, false}] = C64_KEYCODE_COLON;
    mapping[{0x3b, false, false}] = C64_KEYCODE_SEMICOLON;
    mapping[{0x22, false, false}] = C64_KEYCODE_QUOTEDBL;
    mapping[{0x25, false, false}] = C64_KEYCODE_PERCENT;
    mapping[{0x26, false, false}] = C64_KEYCODE_AMPERSAND;
    mapping[{0x28, false, false}] = C64_KEYCODE_LEFTPAREN;
    mapping[{0x29, false, false}] = C64_KEYCODE_RIGHTPAREN;

    mapping[{0x32, false, true}] = C64_KEYCODE_AT;
    mapping[{0x33, false, true}] = C64_KEYCODE_HASH;
    mapping[{0x23, false, false}] = C64_KEYCODE_HASH;

    mapping[{0x24, false, true}] = C64_KEYCODE_UPARROW;
    mapping[{0x3c, false, true}] = C64_KEYCODE_RIGHTBRACKET;
    mapping[{0x2b, false, true}] = C64_KEYCODE_LEFTBRACKET;
  }
};

#endif
#endif