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
#ifndef EXTERNALCMDS_H
#define EXTERNALCMDS_H

#include "NotificationStruct.h"
#include "fs/FSDriver.h"
#include <cstdint>

class C64Sys; // forward declaration

class ExternalCmds {
private:
  uint8_t *ram;
  C64Sys *cpu;
  FSDriver *filesys;
  bool sendrawkeycodes;
  uint16_t actaddrreceivecmd;

  void setVarTab(uint16_t addr);
  void setType1Notification();
  void setType2Notification();
  void setType3Notification(uint16_t addr);
  void setType4Notification();
  void setType5Notification(uint8_t batteryVolLow, uint8_t batteryVolHi);
  void setType6Notification(uint8_t value);
  bool isBasicInputMode();
  void dispVolume();

public:
  bool liststartflag;

  NotificationStruct1 type1notification;
  NotificationStruct2 type2notification;
  NotificationStruct3 type3notification;
  NotificationStruct4 type4notification;
  NotificationStruct5 type5notification;
  NotificationStruct6 type6notification;

  void init(uint8_t *ram, C64Sys *cpu);
  uint8_t executeExternalCmd(uint8_t *buffer);
};

#endif // EXTERNALCMDS_H
