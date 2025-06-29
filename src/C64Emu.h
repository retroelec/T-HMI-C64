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
#ifndef C64EMU_H
#define C64EMU_H

#include "CPUC64.h"
#include "ConfigBoard.h"
#include <atomic>

class C64Emu {
private:
  static C64Emu *instance; // needed for wrapper methods
  static void interruptTODFuncWrapper() {
    if (instance != nullptr) {
      instance->interruptTODFunc();
    }
  }
  static void interruptScanKeyboardFuncWrapper() {
    if (instance != nullptr) {
      instance->interruptScanKeyboardFunc();
    }
  }
  static void interruptProfilingBatteryCheckFuncWrapper() {
    if (instance != nullptr) {
      instance->interruptProfilingBatteryCheckFunc();
    }
  }
  static void cpuCodeWrapper(void *parameter) {
    if (instance != nullptr) {
      instance->cpuCode(parameter);
    }
  }

  uint8_t *ram;
  ConfigBoard board;
  VIC vic;

  uint16_t cntSecondsForBatteryCheck;

  hw_timer_t *interruptProfilingBatteryCheck = NULL;
  hw_timer_t *interruptTOD = NULL;
  hw_timer_t *interruptScanKeyboard = NULL;
  TaskHandle_t cpuTask;

  void interruptTODFunc();
  void interruptScanKeyboardFunc();
  void interruptProfilingBatteryCheckFunc();
  void cpuCode(void *parameter);
  void calibrateBattery();

public:
  CPUC64 cpu;
  std::atomic<bool> showperfvalues = false;
  std::atomic<uint8_t> cntRefreshs = 0;
  std::atomic<uint32_t> numofcyclespersecond = 0;
  std::atomic<uint32_t> numofburnedcyclespersecond = 0;

  void setup();
  void loop();
};

#endif // C64EMU_H
