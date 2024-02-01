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
#include "CIA.h"
#include "CPUC64.h"
#include "SDCard.h"
#include "VIC.h"
#include <stdint.h>

class ExternalCmds {
private:
  uint8_t *ram;
  CPUC64 *cpu;
  VIC *vic;
  CIA *cia1;
  CIA *cia2;
  BLEKB *blekb;
  SDCard sdcard;

public:
  enum cmds {
    NOHOSTCMD = 0,
    SETJOYSTICKMODE = 1,
    SETKBJOYSTICKMODE = 2,
    LOAD = 11,
    RECEIVEDATA = 12,
    SHOWREG = 13,
    SHOWMEM = 14,
    RESET = 20,
    TOGGLEVICDRAW = 21
  };
  cmds hostcmdcode;

  void init(uint8_t *ram, CPUC64 *cpu, VIC *vic, CIA *cia1p, CIA *cia2p,
            BLEKB *blekb);
  void checkExternalCmd();
};

#endif // EXTERNALCMDS_H
