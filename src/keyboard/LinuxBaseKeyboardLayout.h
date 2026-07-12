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
#ifndef LINUXBASEKEYBOARDLAYOUT_H
#define LINUXBASEKEYBOARDLAYOUT_H

#include "C64Keycodes.h"
#include "LinuxKeySpec.h"
#include <cstdint>
#include <map>
#include <string>

class LinuxBaseKeyboardLayout {
protected:
  std::map<KeySpec, CodeTripleS> mapping;
  std::string layoutName;

  void addCommonKeys() {
    mapping[{(uint32_t)'a', false, false}] = C64_KEYCODE_A;
    mapping[{(uint32_t)'b', false, false}] = C64_KEYCODE_B;
    mapping[{(uint32_t)'c', false, false}] = C64_KEYCODE_C;
    mapping[{(uint32_t)'d', false, false}] = C64_KEYCODE_D;
    mapping[{(uint32_t)'e', false, false}] = C64_KEYCODE_E;
    mapping[{(uint32_t)'f', false, false}] = C64_KEYCODE_F;
    mapping[{(uint32_t)'g', false, false}] = C64_KEYCODE_G;
    mapping[{(uint32_t)'h', false, false}] = C64_KEYCODE_H;
    mapping[{(uint32_t)'i', false, false}] = C64_KEYCODE_I;
    mapping[{(uint32_t)'j', false, false}] = C64_KEYCODE_J;
    mapping[{(uint32_t)'k', false, false}] = C64_KEYCODE_K;
    mapping[{(uint32_t)'l', false, false}] = C64_KEYCODE_L;
    mapping[{(uint32_t)'m', false, false}] = C64_KEYCODE_M;
    mapping[{(uint32_t)'n', false, false}] = C64_KEYCODE_N;
    mapping[{(uint32_t)'o', false, false}] = C64_KEYCODE_O;
    mapping[{(uint32_t)'p', false, false}] = C64_KEYCODE_P;
    mapping[{(uint32_t)'q', false, false}] = C64_KEYCODE_Q;
    mapping[{(uint32_t)'r', false, false}] = C64_KEYCODE_R;
    mapping[{(uint32_t)'s', false, false}] = C64_KEYCODE_S;
    mapping[{(uint32_t)'t', false, false}] = C64_KEYCODE_T;
    mapping[{(uint32_t)'u', false, false}] = C64_KEYCODE_U;
    mapping[{(uint32_t)'v', false, false}] = C64_KEYCODE_V;
    mapping[{(uint32_t)'w', false, false}] = C64_KEYCODE_W;
    mapping[{(uint32_t)'x', false, false}] = C64_KEYCODE_X;
    mapping[{(uint32_t)'y', false, false}] = C64_KEYCODE_Y;
    mapping[{(uint32_t)'z', false, false}] = C64_KEYCODE_Z;

    mapping[{(uint32_t)'A', false, false}] = C64_KEYCODE_A;
    mapping[{(uint32_t)'B', false, false}] = C64_KEYCODE_B;
    mapping[{(uint32_t)'C', false, false}] = C64_KEYCODE_C;
    mapping[{(uint32_t)'D', false, false}] = C64_KEYCODE_D;
    mapping[{(uint32_t)'E', false, false}] = C64_KEYCODE_E;
    mapping[{(uint32_t)'F', false, false}] = C64_KEYCODE_F;
    mapping[{(uint32_t)'G', false, false}] = C64_KEYCODE_G;
    mapping[{(uint32_t)'H', false, false}] = C64_KEYCODE_H;
    mapping[{(uint32_t)'I', false, false}] = C64_KEYCODE_I;
    mapping[{(uint32_t)'J', false, false}] = C64_KEYCODE_J;
    mapping[{(uint32_t)'K', false, false}] = C64_KEYCODE_K;
    mapping[{(uint32_t)'L', false, false}] = C64_KEYCODE_L;
    mapping[{(uint32_t)'M', false, false}] = C64_KEYCODE_M;
    mapping[{(uint32_t)'N', false, false}] = C64_KEYCODE_N;
    mapping[{(uint32_t)'O', false, false}] = C64_KEYCODE_O;
    mapping[{(uint32_t)'P', false, false}] = C64_KEYCODE_P;
    mapping[{(uint32_t)'Q', false, false}] = C64_KEYCODE_Q;
    mapping[{(uint32_t)'R', false, false}] = C64_KEYCODE_R;
    mapping[{(uint32_t)'S', false, false}] = C64_KEYCODE_S;
    mapping[{(uint32_t)'T', false, false}] = C64_KEYCODE_T;
    mapping[{(uint32_t)'U', false, false}] = C64_KEYCODE_U;
    mapping[{(uint32_t)'V', false, false}] = C64_KEYCODE_V;
    mapping[{(uint32_t)'W', false, false}] = C64_KEYCODE_W;
    mapping[{(uint32_t)'X', false, false}] = C64_KEYCODE_X;
    mapping[{(uint32_t)'Y', false, false}] = C64_KEYCODE_Y;
    mapping[{(uint32_t)'Z', false, false}] = C64_KEYCODE_Z;

    mapping[{(uint32_t)'0', false, false}] = C64_KEYCODE_0;
    mapping[{(uint32_t)'1', false, false}] = C64_KEYCODE_1;
    mapping[{(uint32_t)'2', false, false}] = C64_KEYCODE_2;
    mapping[{(uint32_t)'3', false, false}] = C64_KEYCODE_3;
    mapping[{(uint32_t)'4', false, false}] = C64_KEYCODE_4;
    mapping[{(uint32_t)'5', false, false}] = C64_KEYCODE_5;
    mapping[{(uint32_t)'6', false, false}] = C64_KEYCODE_6;
    mapping[{(uint32_t)'7', false, false}] = C64_KEYCODE_7;
    mapping[{(uint32_t)'8', false, false}] = C64_KEYCODE_8;
    mapping[{(uint32_t)'9', false, false}] = C64_KEYCODE_9;

    mapping[{(uint32_t)' ', false, false}] = C64_KEYCODE_SPACE;
  }

public:
  virtual ~LinuxBaseKeyboardLayout() = default;
  const std::map<KeySpec, CodeTripleS> &getMapping() const { return mapping; }
  std::string getName() const { return layoutName; }
};

#endif