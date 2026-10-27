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
#ifndef EXTERNALCMDS_H
#define EXTERNALCMDS_H

#include "ExtCmd.h"
#include "NotificationStruct.h"
#include <cstdint>

class C64Sys; // forward declaration

#include "platform/PlatformManager.h"

class ExternalCmds {
private:
  uint8_t *ram;
  C64Sys *cpu;
  bool sendrawkeycodes;
  uint16_t actaddrreceivecmd;

  void setType1Notification();
  void setType4Notification();
  void setType5Notification(uint8_t batteryVolLow, uint8_t batteryVolHi);
  void setType6Notification();
  void dispVolume();
  void writeTextToC64Screen(uint16_t addr, int16_t sizebuffer);
  bool isBasicInputMode();
  void setJ1();
  void setJ2();
  void setNoJoy();
  void setJB1();
  void setJB2();
  void setNoJoyB();

public:
  bool liststartflag;

  NotificationStruct1 type1notification;
  NotificationStruct4 type4notification;
  NotificationStruct5 type5notification;
  NotificationStruct6 type6notification;

  void init(uint8_t *ram, C64Sys *cpu);
  void setVarTab(uint16_t addr);
  uint8_t executeNextExternalCmd();
};

#endif // EXTERNALCMDS_H
