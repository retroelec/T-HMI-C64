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

#include "C64Sys.h"
#include "VIC.h"
#include "board/BoardDriver.h"
#include <atomic>

class C64Emu {
private:
  uint8_t *ram;
  BoardDriver *board;
  uint16_t cntSecondsForBatteryCheck;

  void intervalTimerTODFunc();
  void intervalTimerScanKeyboardFunc();
  void intervalTimerProfilingBatteryCheckFunc();
  void cpuCode(void *parameter);
  void calibrateBattery();

public:
  C64Sys cpu;
  std::atomic<bool> showperfvalues = false;
  std::atomic<uint8_t> cntRefreshs = 0;
  std::atomic<uint32_t> numofcyclespersecond = 0;
  std::atomic<uint32_t> numofburnedcyclespersecond = 0;

  void setup();
  void loop();
};

#endif // C64EMU_H
