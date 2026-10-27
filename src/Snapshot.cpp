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
#include "Snapshot.h"

#include "C64Sys.h"
#include "platform/PlatformManager.h"
#include <cstring>

#ifdef USE_SNAPSHOT

static const char *TAG = "Snapshot";

void Snapshot::save(C64Sys *cpu) {
  PlatformManager::getInstance().log(LOG_INFO, TAG, "save snapshot (%d KB)",
                                     65536 / 1024);
  std::memcpy(ram, cpu->getRamPtr(), 65536);
  cpu_pc = cpu->getPC();
  cpu_a = cpu->getA();
  cpu_x = cpu->getX();
  cpu_y = cpu->getY();
  cpu_sp = cpu->getSP();
  cpu_sr = cpu->getSR();

  std::memcpy(vicRegs, cpu->vic.vicreg, 0x40);
  vicBitmapStart = cpu->vic.bitmapstart;
  vicScreenMemStart = cpu->vic.screenmemstart;

  std::memcpy(cia1Regs, cpu->cia1.ciareg, 0x10);
  cia1TimerA = cpu->cia1.timerA;
  cia1TimerB = cpu->cia1.timerB;
  cia1UnderflowTimerA = cpu->cia1.underflowTimerA;

  std::memcpy(cia2Regs, cpu->cia2.ciareg, 0x10);
  cia2TimerA = cpu->cia2.timerA;
  cia2TimerB = cpu->cia2.timerB;
  cia2UnderflowTimerA = cpu->cia2.underflowTimerA;

  std::memcpy(sidRegs, cpu->sid.sidreg, 0x20);
  sidEmuVolumeScaled = cpu->sid.getEmuVolume();

  joystickMode = cpu->joystickmode;
  kbJoystickMode = cpu->kbjoystickmode;
}

void Snapshot::load(C64Sys *cpu) {
  PlatformManager::getInstance().log(LOG_INFO, TAG, "load snapshot");
  std::memcpy(cpu->getRamPtr(), ram, 65536);
  cpu->setPC(cpu_pc);
  cpu->setA(cpu_a);
  cpu->setX(cpu_x);
  cpu->setY(cpu_y);
  cpu->setSP(cpu_sp);
  cpu->setSR(cpu_sr);

  std::memcpy(cpu->vic.vicreg, vicRegs, 0x40);
  cpu->vic.bitmapstart = vicBitmapStart;
  cpu->vic.screenmemstart = vicScreenMemStart;

  std::memcpy(cpu->cia1.ciareg, cia1Regs, 0x10);
  cpu->cia1.timerA = cia1TimerA;
  cpu->cia1.timerB = cia1TimerB;
  cpu->cia1.underflowTimerA = cia1UnderflowTimerA;

  std::memcpy(cpu->cia2.ciareg, cia2Regs, 0x10);
  cpu->cia2.timerA = cia2TimerA;
  cpu->cia2.timerB = cia2TimerB;
  cpu->cia2.underflowTimerA = cia2UnderflowTimerA;

  std::memcpy(cpu->sid.sidreg, sidRegs, 0x20);
  cpu->sid.setEmuVolume(sidEmuVolumeScaled);
  // c64Volume is derived from sidreg[0x18] & 0x0f on next C64 write – no
  // restore needed.

  // Re-init SID voices to prevent audio distortion after snapshot restore.
  // Voice objects have internal oscillator/envelope state that isn't in the
  // snapshot – resetVoices() resets them cleanly without clearing sidreg[].
  cpu->sid.resetVoices();

  cpu->joystickmode = joystickMode;
  cpu->kbjoystickmode = kbJoystickMode;
}

#else // !USE_SNAPSHOT

void Snapshot::save(C64Sys *) {}
void Snapshot::load(C64Sys *) {}

#endif // USE_SNAPSHOT
