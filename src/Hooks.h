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
#ifndef HOOKS_H
#define HOOKS_H

#include <cstdint>

class C64Sys; // forward declaration

class Hooks {
private:
  uint8_t *ram;
  C64Sys *cpu;

public:
  void init(uint8_t *ram, C64Sys *cpu);
  void patchKernal(uint8_t *kernal_rom);
  bool handlehooks(uint16_t pc);
};

#endif // HOOKS_H
