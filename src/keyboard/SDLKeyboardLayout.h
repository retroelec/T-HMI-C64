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
#ifndef SDLKEYBOARDLAYOUT_H
#define SDLKEYBOARDLAYOUT_H

#include "../Config.h"
#ifdef USE_SDL_KEYBOARD

#include "C64Keycodes.h"
#include "SDLKeySpec.h"
#include <map>
#include <string>

class SDLKeyboardLayout {
protected:
  std::map<KeySpec, CodeTripleS> mapping;
  std::string layoutName;

  void addCommonKeys() {
    mapping[{SDLK_a, false, false}] = C64_KEYCODE_A;
    mapping[{SDLK_b, false, false}] = C64_KEYCODE_B;
    mapping[{SDLK_c, false, false}] = C64_KEYCODE_C;
    mapping[{SDLK_d, false, false}] = C64_KEYCODE_D;
    mapping[{SDLK_e, false, false}] = C64_KEYCODE_E;
    mapping[{SDLK_f, false, false}] = C64_KEYCODE_F;
    mapping[{SDLK_g, false, false}] = C64_KEYCODE_G;
    mapping[{SDLK_h, false, false}] = C64_KEYCODE_H;
    mapping[{SDLK_i, false, false}] = C64_KEYCODE_I;
    mapping[{SDLK_j, false, false}] = C64_KEYCODE_J;
    mapping[{SDLK_k, false, false}] = C64_KEYCODE_K;
    mapping[{SDLK_l, false, false}] = C64_KEYCODE_L;
    mapping[{SDLK_m, false, false}] = C64_KEYCODE_M;
    mapping[{SDLK_n, false, false}] = C64_KEYCODE_N;
    mapping[{SDLK_o, false, false}] = C64_KEYCODE_O;
    mapping[{SDLK_p, false, false}] = C64_KEYCODE_P;
    mapping[{SDLK_q, false, false}] = C64_KEYCODE_Q;
    mapping[{SDLK_r, false, false}] = C64_KEYCODE_R;
    mapping[{SDLK_s, false, false}] = C64_KEYCODE_S;
    mapping[{SDLK_t, false, false}] = C64_KEYCODE_T;
    mapping[{SDLK_u, false, false}] = C64_KEYCODE_U;
    mapping[{SDLK_v, false, false}] = C64_KEYCODE_V;
    mapping[{SDLK_w, false, false}] = C64_KEYCODE_W;
    mapping[{SDLK_x, false, false}] = C64_KEYCODE_X;
    mapping[{SDLK_y, false, false}] = C64_KEYCODE_Y;
    mapping[{SDLK_z, false, false}] = C64_KEYCODE_Z;

    mapping[{SDLK_0, false, false}] = C64_KEYCODE_0;
    mapping[{SDLK_1, false, false}] = C64_KEYCODE_1;
    mapping[{SDLK_2, false, false}] = C64_KEYCODE_2;
    mapping[{SDLK_3, false, false}] = C64_KEYCODE_3;
    mapping[{SDLK_4, false, false}] = C64_KEYCODE_4;
    mapping[{SDLK_5, false, false}] = C64_KEYCODE_5;
    mapping[{SDLK_6, false, false}] = C64_KEYCODE_6;
    mapping[{SDLK_7, false, false}] = C64_KEYCODE_7;
    mapping[{SDLK_8, false, false}] = C64_KEYCODE_8;
    mapping[{SDLK_9, false, false}] = C64_KEYCODE_9;

    mapping[{SDLK_RETURN, false, false}] = C64_KEYCODE_RETURN;
    mapping[{SDLK_SPACE, false, false}] = C64_KEYCODE_SPACE;
    mapping[{SDLK_BACKSPACE, false, false}] = C64_KEYCODE_BACKSPACE;
    mapping[{SDLK_ESCAPE, false, false}] = C64_KEYCODE_BREAK;
    mapping[{SDLK_UP, false, false}] = C64_KEYCODE_UP;
    mapping[{SDLK_DOWN, false, false}] = C64_KEYCODE_DOWN;
    mapping[{SDLK_LEFT, false, false}] = C64_KEYCODE_LEFT;
    mapping[{SDLK_RIGHT, false, false}] = C64_KEYCODE_RIGHT;
    mapping[{SDLK_F1, false, false}] = C64_KEYCODE_F1;
    mapping[{SDLK_F3, false, false}] = C64_KEYCODE_F3;
    mapping[{SDLK_F5, false, false}] = C64_KEYCODE_F5;
    mapping[{SDLK_F7, false, false}] = C64_KEYCODE_F7;
    mapping[{SDLK_HOME, false, false}] = C64_KEYCODE_HOME;
    mapping[{SDLK_DELETE, false, false}] = C64_KEYCODE_DEL;
    mapping[{SDLK_HOME, true, false}] = C64_KEYCODE_CLR;
    mapping[{SDLK_DELETE, true, false}] = C64_KEYCODE_INS;

    mapping[{SDLK_LSHIFT, false, false}] = C64_KEYCODE_SHIFTLEFT;
    mapping[{SDLK_RSHIFT, false, false}] = C64_KEYCODE_SHIFTRIGHT;
    mapping[{SDLK_LALT, false, false}] = C64_KEYCODE_COMMODORE;
    mapping[{SDLK_LCTRL, false, false}] = C64_KEYCODE_CTRL;
  }

public:
  virtual ~SDLKeyboardLayout() {}
  const std::map<KeySpec, CodeTripleS> &getMapping() const { return mapping; }
  std::string getName() const { return layoutName; }
};

#endif
#endif
