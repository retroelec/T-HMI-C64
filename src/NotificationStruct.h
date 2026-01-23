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
#ifndef NOTIFICATIONSTRUCT_H
#define NOTIFICATIONSTRUCT_H

#include <cstdint>

// notifications may be not larger than 20 bytes

struct NotificationStruct {
  uint8_t type;
};

struct NotificationStruct1 : NotificationStruct {
  uint8_t joymode;
  uint8_t deactivateTemp;
  uint8_t sendrawkeycodes;
  uint8_t switchdebug;
  uint8_t switchperf;
  uint8_t switchdetectreleasekey;
  uint8_t volume;
  uint8_t switchattached;
};

struct NotificationStruct2 : NotificationStruct {
  uint8_t cpuRunning;
  uint16_t pc;
  uint8_t a;
  uint8_t x;
  uint8_t y;
  uint8_t sr;
  uint8_t d011;
  uint8_t d016;
  uint8_t d018;
  uint8_t d019;
  uint8_t d01a;
  uint8_t register1;
  uint8_t dc0d;
  uint8_t dc0e;
  uint8_t dc0f;
  uint8_t dd0d;
  uint8_t dd0e;
  uint8_t dd0f;
};

static const uint8_t NOTIFICATIONTYPE3NUMOFBYTES = 16; // must be divisible by 8

struct NotificationStruct3 : NotificationStruct {
  uint8_t mem[NOTIFICATIONTYPE3NUMOFBYTES];
};

struct NotificationStruct4 : NotificationStruct {};

struct NotificationStruct5 : NotificationStruct {
  uint8_t batteryVolLow;
  uint8_t batteryVolHi;
};

#endif // NOTIFICATIONSTRUCT_H
