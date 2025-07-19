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
#ifndef SDCARD_H
#define SDCARD_H

#include "../Config.h"
#ifdef USE_SDCARD
#include "FSDriver.h"
#include <FS.h>
#include <cstdint>

class SDCard : public FSDriver {
private:
  bool initalized;
  File root;

public:
  SDCard();
  bool init() override;
  uint16_t load(char *path, uint8_t *ram) override;
  bool save(char *path, uint8_t *ram, uint16_t startaddr,
            uint16_t endaddr) override;
  bool listnextentry(uint8_t *nextentry, bool start) override;
};
#endif

#endif // SDCARD_H
