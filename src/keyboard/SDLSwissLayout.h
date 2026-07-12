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
#ifndef SDLSWISSLAYOUT_H
#define SDLSWISSLAYOUT_H

#include "../Config.h"
#ifdef USE_SDL_KEYBOARD

#include "SDLKeyboardLayout.h"

class SDLSwissLayout : public SDLKeyboardLayout {
public:
  SDLSwissLayout() {
    layoutName = "Swiss";
    addCommonKeys();

    mapping[{(uint32_t)SDLK_MINUS, false, false}] = C64_KEYCODE_MINUS;
    mapping[{(uint32_t)SDLK_PERIOD, false, false}] = C64_KEYCODE_PERIOD;
    mapping[{(uint32_t)SDLK_COMMA, false, false}] = C64_KEYCODE_COMMA;
    mapping[{(uint32_t)SDLK_DOLLAR, false, false}] = C64_KEYCODE_DOLLAR;
    mapping[{(uint32_t)SDLK_LESS, false, false}] = C64_KEYCODE_LESS;
    mapping[{(uint32_t)SDLK_QUOTE, false, false}] = C64_KEYCODE_QUOTE;

    mapping[{(uint32_t)SDLK_LESS, true, false}] = C64_KEYCODE_GREATER;
    mapping[{(uint32_t)SDLK_QUOTE, true, false}] = C64_KEYCODE_QUESTION;
    mapping[{(uint32_t)SDLK_1, true, false}] = C64_KEYCODE_PLUS;
    mapping[{(uint32_t)SDLK_3, true, false}] = C64_KEYCODE_ASTERISK;
    mapping[{(uint32_t)SDLK_7, true, false}] = C64_KEYCODE_SLASH;
    mapping[{(uint32_t)SDLK_0, true, false}] = C64_KEYCODE_EQUALS;
    mapping[{(uint32_t)SDLK_PERIOD, true, false}] = C64_KEYCODE_COLON;
    mapping[{(uint32_t)SDLK_COMMA, true, false}] = C64_KEYCODE_SEMICOLON;
    mapping[{(uint32_t)SDLK_DOLLAR, true, false}] = C64_KEYCODE_POUND;
    mapping[{(uint32_t)SDLK_2, true, false}] = C64_KEYCODE_QUOTEDBL;
    mapping[{(uint32_t)SDLK_5, true, false}] = C64_KEYCODE_PERCENT;
    mapping[{(uint32_t)SDLK_6, true, false}] = C64_KEYCODE_AMPERSAND;
    mapping[{(uint32_t)SDLK_8, true, false}] = C64_KEYCODE_LEFTPAREN;
    mapping[{(uint32_t)SDLK_9, true, false}] = C64_KEYCODE_RIGHTPAREN;

    mapping[{(uint32_t)SDLK_2, false, true}] = C64_KEYCODE_AT;
    mapping[{(uint32_t)SDLK_3, false, true}] = C64_KEYCODE_HASH;

    mapping[{1073741824, true, false}] = C64_KEYCODE_EXCLAIM;
    mapping[{(uint32_t)SDLK_DOLLAR, false, true}] = C64_KEYCODE_UPARROW;
    mapping[{1073741824, false, false}] = C64_KEYCODE_LEFTARROW;
    mapping[{252, false, true}] = C64_KEYCODE_LEFTBRACKET;
    mapping[{1073741824, false, true}] = C64_KEYCODE_RIGHTBRACKET;
  }
};

#endif
#endif
