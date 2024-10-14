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
#ifndef C64EMU_H
#define C64EMU_H

#include "BLEKB.h"
#include "CPUC64.h"
#include "ConfigBoard.h"
#include "ExternalCmds.h"

class C64Emu {
private:
  static C64Emu *instance; // needed for wrapper methods
  static void interruptTODFuncWrapper() {
    if (instance != nullptr) {
      instance->interruptTODFunc();
    }
  }
  static void interruptSystemFuncWrapper() {
    if (instance != nullptr) {
      instance->interruptSystemFunc();
    }
  }
  static void interruptProfilingFuncWrapper() {
    if (instance != nullptr) {
      instance->interruptProfilingFunc();
    }
  }
  static void cpuCodeWrapper(void *parameter) {
    if (instance != nullptr) {
      instance->cpuCode(parameter);
    }
  }

  uint8_t *ram;
  ConfigBoard configBoard;
  VIC vic;

  uint16_t checkForKeyboardCnt = 0;
  uint8_t throttlingCnt = 0;
  uint32_t numofburnedcyclespersecond = 0;

  hw_timer_t *interruptProfiling = NULL;
  hw_timer_t *interruptTOD = NULL;
  hw_timer_t *interruptSystem = NULL;
  TaskHandle_t cpuTask;

  void IRAM_ATTR interruptTODFunc();
  void IRAM_ATTR interruptSystemFunc();
  void IRAM_ATTR interruptProfilingFunc();
  void cpuCode(void *parameter);
  bool updateTOD(CIA &cia);

public:
  CPUC64 cpu;
  BLEKB blekb;
  ExternalCmds externalCmds;

  void setup();
  void loop();
};

#endif // C64EMU_H
