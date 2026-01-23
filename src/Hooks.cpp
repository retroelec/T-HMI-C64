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
#include "Hooks.h"

#include "C64Sys.h"
#include "platform/PlatformManager.h"
#include <cstdint>

static const char *TAG = "Hooks";

static const uint16_t IECINHOOK = 0xee13;
static const uint16_t IECOUTHOOK = 0xed40;
static const uint16_t IECWAIT4CLKHOOK = 0xedcc;

void Hooks::init(uint8_t *ram, C64Sys *cpu) {
  this->ram = ram;
  this->cpu = cpu;
}

void Hooks::patchKernal(uint8_t *kernal_rom) {
  // patch kernal_rom
  kernal_rom[IECINHOOK - 0xe000] = 0;
  kernal_rom[IECOUTHOOK - 0xe000] = 0;
  kernal_rom[IECWAIT4CLKHOOK - 0xe000] = 0;
}

bool Hooks::handlehooks(uint16_t pc) {
  if (pc == IECINHOOK + 1) {
    uint8_t a = cpu->floppy.iecin();
    // PlatformManager::getInstance().log(LOG_INFO, TAG, "iecin hook: %x", a);
    cpu->setA(a);
    ram[0xa4] = a;
    ram[0xa5] = 0;
    ram[0x90] = cpu->floppy.lastStatus;
    cpu->setPC(0xee82);
    return true;
  } else if (pc == IECOUTHOOK + 1) {
    uint8_t a = ram[0x95];
    PlatformManager::getInstance().log(LOG_INFO, TAG, "iecout hook: %x", a);
    cpu->floppy.iecout(a);
    ram[0xa5] = 0;
    ram[0x90] = cpu->floppy.lastStatus;
    cpu->setPC(0xee82);
    return true;
  } else if (pc == IECWAIT4CLKHOOK + 1) {
    PlatformManager::getInstance().log(LOG_INFO, TAG, "wait4clk hook");
    cpu->setPC(0xeddb);
    return true;
  }
  return false;
}
