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

#include "BLEKB.h"
#include "CPUC64.h"
#include "SDCard.h"
#include <cstdint>

class ExternalCmds {
private:
  uint8_t *ram;
  CPUC64 *cpu;
  BLEKB *blekb;
  SDCard sdcard;

public:
  enum cmds {
    NOHOSTCMD = 0,
    JOYSTICKMODE1 = 1,
    JOYSTICKMODE2 = 2,
    KBJOYSTICKMODE1 = 3,
    KBJOYSTICKMODE2 = 4,
    JOYSTICKMODEOFF = 5,
    KBJOYSTICKMODEOFF = 6,
    LOAD = 11,
    RECEIVEDATA = 12,
    SHOWREG = 13,
    SHOWMEM = 14,
    RESET = 20,
    TOGGLEVICDRAW = 21,
    TOGGLECIA2 = 22
  };
  cmds hostcmdcode;

  void init(uint8_t *ram, CPUC64 *cpu, BLEKB *blekb);
  void checkExternalCmd();
};

#endif // EXTERNALCMDS_H
