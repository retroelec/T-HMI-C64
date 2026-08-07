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
#ifndef TERMINALTEXTKEYBOARDLAYOUT_H
#define TERMINALTEXTKEYBOARDLAYOUT_H

#include "../Config.h"
#ifdef USE_TERMINALTEXT_KEYBOARD

#include "LinuxBaseKeyboardLayout.h"

// Terminal keycodes for special keys (values > 0xFF to avoid ASCII conflicts)
#define TKEY_UP 0x1001
#define TKEY_DOWN 0x1002
#define TKEY_LEFT 0x1003
#define TKEY_RIGHT 0x1004
#define TKEY_F1 0x1005
#define TKEY_F3 0x1006
#define TKEY_F5 0x1007
#define TKEY_F7 0x1008
#define TKEY_HOME 0x1009
#define TKEY_DEL 0x100A
#define TKEY_INS 0x100B
#define TKEY_ENTER 0x100C
#define TKEY_BACKSPACE 0x100D
#define TKEY_TAB 0x100E
#define TKEY_ESC 0x100F

class TerminalTextKeyboardLayout : public LinuxBaseKeyboardLayout {
public:
  TerminalTextKeyboardLayout() { addCommonKeys(); }

  void addCommonKeys() {
    LinuxBaseKeyboardLayout::addCommonKeys();

    mapping[{(uint32_t)TKEY_ENTER, false, false}] = C64_KEYCODE_RETURN;
    mapping[{(uint32_t)TKEY_BACKSPACE, false, false}] = C64_KEYCODE_BACKSPACE;
    mapping[{(uint32_t)TKEY_ESC, false, false}] = C64_KEYCODE_BREAK;

    mapping[{(uint32_t)TKEY_UP, false, false}] = C64_KEYCODE_UP;
    mapping[{(uint32_t)TKEY_DOWN, false, false}] = C64_KEYCODE_DOWN;
    mapping[{(uint32_t)TKEY_LEFT, false, false}] = C64_KEYCODE_LEFT;
    mapping[{(uint32_t)TKEY_RIGHT, false, false}] = C64_KEYCODE_RIGHT;

    mapping[{(uint32_t)TKEY_F1, false, false}] = C64_KEYCODE_F1;
    mapping[{(uint32_t)TKEY_F3, false, false}] = C64_KEYCODE_F3;
    mapping[{(uint32_t)TKEY_F5, false, false}] = C64_KEYCODE_F5;
    mapping[{(uint32_t)TKEY_F7, false, false}] = C64_KEYCODE_F7;

    mapping[{(uint32_t)TKEY_HOME, false, false}] = C64_KEYCODE_HOME;
    mapping[{(uint32_t)TKEY_DEL, false, false}] = C64_KEYCODE_DEL;
    mapping[{(uint32_t)TKEY_INS, false, false}] = C64_KEYCODE_INS;

    // common punctuation (unshifted)
    mapping[{(uint32_t)0x2d, false, false}] = C64_KEYCODE_MINUS;
    mapping[{(uint32_t)0x2e, false, false}] = C64_KEYCODE_PERIOD;
    mapping[{(uint32_t)0x2c, false, false}] = C64_KEYCODE_COMMA;
    mapping[{(uint32_t)0x27, false, false}] = C64_KEYCODE_QUOTE;
    mapping[{(uint32_t)0x3b, false, false}] = C64_KEYCODE_SEMICOLON;
    mapping[{(uint32_t)0x2f, false, false}] = C64_KEYCODE_SLASH;
    mapping[{(uint32_t)0x3d, false, false}] = C64_KEYCODE_EQUALS;
    mapping[{(uint32_t)0x5b, false, false}] = C64_KEYCODE_COLON;
    mapping[{(uint32_t)0x5d, false, false}] = C64_KEYCODE_SEMICOLON;
    mapping[{(uint32_t)0x60, false, false}] = C64_KEYCODE_UPARROW;

    // common shifted punctuation (works on DE/CH/US layouts)
    mapping[{(uint32_t)0x24, false, false}] = C64_KEYCODE_DOLLAR;
    mapping[{(uint32_t)0x3c, false, false}] = C64_KEYCODE_LESS;
    mapping[{(uint32_t)0x2b, false, false}] = C64_KEYCODE_PLUS;
    mapping[{(uint32_t)0x2a, false, false}] = C64_KEYCODE_ASTERISK;
    mapping[{(uint32_t)0x3a, false, false}] = C64_KEYCODE_COLON;
    mapping[{(uint32_t)0x22, false, false}] = C64_KEYCODE_QUOTEDBL;
    mapping[{(uint32_t)0x25, false, false}] = C64_KEYCODE_PERCENT;
    mapping[{(uint32_t)0x26, false, false}] = C64_KEYCODE_AMPERSAND;
    mapping[{(uint32_t)0x28, false, false}] = C64_KEYCODE_LEFTPAREN;
    mapping[{(uint32_t)0x29, false, false}] = C64_KEYCODE_RIGHTPAREN;
    mapping[{(uint32_t)0x3f, false, false}] = C64_KEYCODE_QUESTION;
    mapping[{(uint32_t)0x23, false, false}] = C64_KEYCODE_HASH;

    // altgr combinations for brackets and special chars
    mapping[{(uint32_t)0x32, false, true}] = C64_KEYCODE_AT;
    mapping[{(uint32_t)0x33, false, true}] = C64_KEYCODE_HASH;
    mapping[{(uint32_t)0x24, false, true}] = C64_KEYCODE_UPARROW;
    mapping[{(uint32_t)0x3c, false, true}] = C64_KEYCODE_RIGHTBRACKET;
    mapping[{(uint32_t)0x2b, false, true}] = C64_KEYCODE_LEFTBRACKET;
  }

public:
  virtual uint32_t normalizeKey(uint32_t ch) const {
    if (ch >= 'A' && ch <= 'Z') {
      return ch + 0x20;
    }
    return ch;
  }

  virtual ~TerminalTextKeyboardLayout() {}
};

#endif
#endif // TERMINALTEXTKEYBOARDLAYOUT_H