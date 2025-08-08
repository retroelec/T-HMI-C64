#ifndef SDLKEYMAP_H
#define SDLKEYMAP_H

#include "../Config.h"
#ifdef USE_SDL_KEYBOARD
#include <SDL2/SDL.h>
#include <map>
#include <tuple>

using KeySpec = std::tuple<SDL_Keycode, bool, bool>; // key, shift, ctrl
using CodeTriple = std::tuple<uint8_t, uint8_t, uint8_t>;

extern const std::map<KeySpec, CodeTriple> keyMap;
#endif

#endif // SDLKEYMAP_H
