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
#include "../Config.h"
#ifdef USE_LINUXFS
#include "../platform/PlatformManager.h"
#include "LinuxFS.h"
#include <cstring>
#include <fstream>
#include <iostream>

static const char *TAG = "LinuxFS";

bool LinuxFS::init() { return true; }

uint16_t LinuxFS::load(char *filename, uint8_t *ram) {
  char path[64];
  strcpy(path, Config::PATH);
  strcat(path, filename);
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "cannot open file %s",
                                       path);
    return 0;
  }
  uint8_t addrLow = file.get();
  uint8_t addrHigh = file.get();
  if (file.eof()) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "not a prg file");
    return 0;
  }
  uint16_t addr = addrLow | (addrHigh << 8);
  char byte;
  while (file.get(byte)) {
    ram[addr++] = static_cast<uint8_t>(byte);
  }
  file.close();
  return addr;
}

bool LinuxFS::save(char *filename, uint8_t *ram, uint16_t startaddr,
                   uint16_t endaddr) {
  return false;
}

bool LinuxFS::listnextentry(uint8_t *nextentry, bool start) { return false; }
#endif
