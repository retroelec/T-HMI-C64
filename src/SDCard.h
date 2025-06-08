/*
 Copyright (C) 2024 retroelec <retroelec42@gmail.com>

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

#include <FS.h>
#include <cstdint>

class SDCard {
private:
  bool initalized;
  File root;

public:
  SDCard();
  bool init();
  uint16_t load(uint8_t *ram);
  bool save(uint8_t *ram);
  bool listnextentry(uint8_t *nextentry, bool start);
};

#endif // SDCARD_H
