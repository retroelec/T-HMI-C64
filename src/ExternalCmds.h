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
#ifndef EXTERNALCMDS_H
#define EXTERNALCMDS_H

#include "ExtCmdEnum.h"
#include "SDCard.h"
#include <cstdint>

struct BLENotificationStruct1 {
  uint8_t type;
  uint8_t joymode;
  uint8_t kbjoymode;
  uint8_t refreshframecolor;
  uint8_t switchonoffcia2;
  uint8_t joyemulmode;
  uint8_t cpuRunning;
};

struct BLENotificationStruct2 {
  uint8_t type;
  uint16_t pc;
  uint8_t a;
  uint8_t x;
  uint8_t y;
  uint8_t sr;
  uint8_t d011;
  uint8_t d016;
  uint8_t d018;
  uint8_t d019;
  uint8_t d010;
};

class ExternalCmds {
private:
  uint8_t *ram;
  SDCard sdcard;

public:
  BLENotificationStruct1 type1notification;
  BLENotificationStruct2 type2notification;

  void init(uint8_t *ram);
  void setType1Notification();
  void setType2Notification();
  uint8_t executeExternalCmd(ExtCmd cmd);
};

#endif // EXTERNALCMDS_H
