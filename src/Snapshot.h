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
#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include "Config.h"
#include <cstdint>

class C64Sys; // forward declaration

/**
 * @brief Save and restore a full emulator snapshot.
 *
 * Captures RAM (64 KB), CPU registers, VIC, CIA and SID state.
 * Only active when USE_SNAPSHOT is defined (desktop builds).
 * On ESP32 this uses minimal memory and does nothing.
 */
class Snapshot {
public:
  /** Save current emulator state into internal buffer. */
  void save(C64Sys *cpu);

  /** Restore emulator state from internal buffer. */
  void load(C64Sys *cpu);

private:
#ifdef USE_SNAPSHOT
  // ---- snapshot data (only present on desktop builds) ----
  uint8_t ram[65536];
  // CPU
  uint16_t cpu_pc;
  uint8_t cpu_a, cpu_x, cpu_y, cpu_sp, cpu_sr;
  // VIC
  uint8_t vicRegs[0x40];
  uint16_t vicBitmapStart;
  uint16_t vicScreenMemStart;
  // CIA1 + CIA2
  uint8_t cia1Regs[0x10];
  uint16_t cia1TimerA, cia1TimerB;
  bool cia1UnderflowTimerA;
  uint8_t cia2Regs[0x10];
  uint16_t cia2TimerA, cia2TimerB;
  bool cia2UnderflowTimerA;
  // SID
  uint8_t sidRegs[0x20];
  uint8_t sidEmuVolumeScaled;
  // C64Sys mode flags
  uint8_t joystickMode;
  uint8_t kbJoystickMode;
#endif // USE_SNAPSHOT
};

#endif // SNAPSHOT_H