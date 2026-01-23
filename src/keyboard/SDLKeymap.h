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
#ifndef SDLKEYMAP_H
#define SDLKEYMAP_H

#include "../Config.h"
#ifdef USE_SDL_KEYBOARD
#include "C64Keycodes.h"
#include "CodeTripleDef.h"
#include <SDL2/SDL.h>
#include <map>
#include <tuple>

using KeySpec = std::tuple<SDL_Keycode, bool, bool>; // key, shift, ralt

extern const std::map<KeySpec, CodeTripleS> keyMap;
#endif

#endif // SDLKEYMAP_H
