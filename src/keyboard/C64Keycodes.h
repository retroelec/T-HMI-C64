/*
 Copyright (C) 2024-2025 retroelec <retroelec42@gmail.com>

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
#ifndef C64KEYCODES_H
#define C64KEYCODES_H

#include "CodeTripleDef.h"

#define X(name, a, b, c)                                                       \
  inline constexpr CodeTripleS C64_KEYCODE_##name{a, b, c};
#include "C64Keycodes_inc.h"
#undef X

struct NamedKeycode {
  const char *name;
  CodeTripleS value;
};

inline constexpr NamedKeycode C64_KEYCODES[] = {
#define X(name, a, b, c) {"C64_KEYCODE_" #name, {a, b, c}},
#include "C64Keycodes_inc.h"
#undef X
};

#endif // C64KEYCODES_H
