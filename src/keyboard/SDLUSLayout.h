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
#ifndef SDLUSLAYOUT_H
#define SDLUSLAYOUT_H

#include "../Config.h"
#ifdef USE_SDL_KEYBOARD

#include "SDLKeyboardLayout.h"

class SDLUSLayout : public SDLKeyboardLayout {
public:
  SDLUSLayout() {
    layoutName = "US";
    addCommonKeys();

    mapping[{SDLK_MINUS, false, false}] = C64_KEYCODE_MINUS;
    mapping[{SDLK_PERIOD, false, false}] = C64_KEYCODE_PERIOD;
    mapping[{SDLK_COMMA, false, false}] = C64_KEYCODE_COMMA;
    mapping[{SDLK_4, true, false}] = C64_KEYCODE_DOLLAR;
    mapping[{SDLK_COMMA, true, false}] = C64_KEYCODE_LESS;
    mapping[{SDLK_QUOTE, false, false}] = C64_KEYCODE_QUOTE;

    mapping[{SDLK_PERIOD, true, false}] = C64_KEYCODE_GREATER;
    mapping[{SDLK_SLASH, true, false}] = C64_KEYCODE_QUESTION;
    mapping[{SDLK_EQUALS, true, false}] = C64_KEYCODE_PLUS;
    mapping[{SDLK_8, true, false}] = C64_KEYCODE_ASTERISK;
    mapping[{SDLK_SLASH, false, false}] = C64_KEYCODE_SLASH;
    mapping[{SDLK_EQUALS, false, false}] = C64_KEYCODE_EQUALS;
    mapping[{SDLK_SEMICOLON, true, false}] = C64_KEYCODE_COLON;
    mapping[{SDLK_SEMICOLON, false, false}] = C64_KEYCODE_SEMICOLON;
    mapping[{SDLK_MINUS, true, false}] = C64_KEYCODE_POUND;
    mapping[{SDLK_QUOTE, true, false}] = C64_KEYCODE_QUOTEDBL;
    mapping[{SDLK_5, true, false}] = C64_KEYCODE_PERCENT;
    mapping[{SDLK_7, true, false}] = C64_KEYCODE_AMPERSAND;
    mapping[{SDLK_9, true, false}] = C64_KEYCODE_LEFTPAREN;
    mapping[{SDLK_0, true, false}] = C64_KEYCODE_RIGHTPAREN;

    mapping[{SDLK_2, true, false}] = C64_KEYCODE_AT;
    mapping[{SDLK_3, true, false}] = C64_KEYCODE_HASH;

    mapping[{SDLK_1, true, false}] = C64_KEYCODE_EXCLAIM;
    mapping[{SDLK_6, true, false}] = C64_KEYCODE_UPARROW;
    mapping[{SDLK_BACKQUOTE, false, false}] = C64_KEYCODE_LEFTARROW;
    mapping[{SDLK_LEFTBRACKET, false, false}] = C64_KEYCODE_LEFTBRACKET;
    mapping[{SDLK_RIGHTBRACKET, false, false}] = C64_KEYCODE_RIGHTBRACKET;
  }
};

#endif
#endif
