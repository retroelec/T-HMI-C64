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
#include "ExternalCmds.h"

#include <ArduinoLog.h>
#include <chrono>
#include <thread>

void ExternalCmds::init(uint8_t *ramp, CPUC64 *cpup, VIC *vicp, CIA *cia1p,
                        CIA *cia2p, BLEKB *blekbp) {
  ram = ramp;
  cpu = cpup;
  vic = vicp;
  cia1 = cia1p;
  cia2 = cia2p;
  blekb = blekbp;
  hostcmdcode = NOHOSTCMD;
}

void ExternalCmds::checkExternalCmd() {
  uint8_t low;
  uint8_t hi;
  uint8_t numofbytes;
  uint16_t addr;
  switch (ExternalCmds::hostcmdcode) {
  case NOHOSTCMD:
    return;
  case LOAD:
    Log.noticeln("load from sdcard...");
    cpu->cpuhalted.store(true, std::memory_order_release);
    if (sdcard.init()) {
      uint8_t cury = ram[0xd6];
      uint8_t curx = ram[0xd3];
      addr = sdcard.load(SD_MMC, ram + 0x0400 + cury * 40 + curx, ram);
      if (addr == 0) {
        Log.noticeln("error loading file");
      } else {
        // set VARTAB
        ram[0x2d] = addr % 256;
        ram[0x2e] = addr / 256;
        // clr
        cpu->setPC(0xa52a);
      }
    } else {
      Log.noticeln("error init sdcard");
    }
    cpu->cpuhalted.store(false, std::memory_order_release);
    break;
  case RECEIVEDATA:
    Log.noticeln("enter hostcmd_receivedata");
    cpu->cpuhalted = true;
    while (true) {
      // simple "protocol":
      // - first byte = number of bytes which will be sent (max 248 bytes)
      // - next two bytes = address the bytes must be written to
      // - afterwards send announced number of bytes
      // - if numofbytes == 255 then transfer is finished -> init basic
      while (!blekb->getData(&numofbytes)) {
      }
      Log.noticeln("number of bytes to transfer: %x", numofbytes);
      if (numofbytes == 255) {
        break;
      }
      while (!blekb->getData(&low)) {
      }
      while (!blekb->getData(&hi)) {
      }
      addr = low + (hi << 8);
      Log.noticeln("address data will be transfered to: %x", low + (hi << 8));
      for (uint8_t i = 0; i < numofbytes; i++) {
        while (!blekb->getData(&ram[addr])) {
        }
        addr++;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    // set VARTAB
    ram[0x2d] = addr % 256;
    ram[0x2e] = addr / 256;
    // clr
    cpu->setPC(0xa52a);
    cpu->cpuhalted = false;
    Log.noticeln("leave hostcmd_receivedata");
    break;
  case SHOWREG:
    Log.noticeln("pc = %x, a = %x, x = %x, y = %x, sr = %B", cpu->getPC(),
                 cpu->getA(), cpu->getX(), cpu->getY(), cpu->getSR());
    Log.noticeln("reg1 = %x, ba = %t, bd = %t, be = %t, dio = %t",
                 cpu->register1, cpu->bankARAM, cpu->bankDRAM, cpu->bankERAM,
                 cpu->bankDIO);
    for (uint8_t i = 0x11; i <= 0x1a; i++) {
      Log.noticeln("vic[%x] = %B", i,
                   vic->vicreg[i].load(std::memory_order_acquire));
    }
    Log.noticeln("l11 = %B, l12 = %B, vicmem = %x",
                 vic->latchd011.load(std::memory_order_acquire),
                 vic->latchd012.load(std::memory_order_acquire),
                 vic->vicmem.load(std::memory_order_acquire));
    for (uint8_t i = 0x04; i <= 0x0f; i++) {
      Log.noticeln("cia1[%x] = %B", i,
                   cia1->ciareg[i].load(std::memory_order_acquire));
    }
    Log.noticeln("l04 = %B, l05 = %B, l06 = %B, l07 = %B, l0d = %B, timerA = "
                 "%x, timerB = %x, reloadA = %t, reloadB = %t",
                 cia1->latchdc04.load(std::memory_order_acquire),
                 cia1->latchdc05.load(std::memory_order_acquire),
                 cia1->latchdc06.load(std::memory_order_acquire),
                 cia1->latchdc07.load(std::memory_order_acquire),
                 cia1->latchdc0d.load(std::memory_order_acquire),
                 cia1->timerA.load(std::memory_order_acquire),
                 cia1->timerB.load(std::memory_order_acquire),
                 cia1->reloadA.load(std::memory_order_acquire),
                 cia1->reloadB.load(std::memory_order_acquire));
    break;
  case SHOWMEM:
    while (!blekb->getData(&numofbytes)) {
    }
    while (!blekb->getData(&low)) {
    }
    while (!blekb->getData(&hi)) {
    }
    addr = low + (hi << 8);
    for (uint8_t i = 0; i < numofbytes; i++) {
      Log.noticeln("ram[%x] = %x", addr, ram[addr]);
      addr++;
    }
    break;
  case RESET:
    cpu->cpuhalted = true;
    vic->vicreg[0x11].store(0x1b, std::memory_order_release);
    vic->vicreg[0x16].store(0xc8, std::memory_order_release);
    vic->vicreg[0x18].store(0x15, std::memory_order_release);
    cia2->ciareg[0x00].store(0x97, std::memory_order_release);
    cpu->setPC(0xfce2);
    cpu->cpuhalted = false;
    break;
  case TOGGLEVICDRAW:
    vic->drawnotevenodd.store(
        !vic->drawnotevenodd.load(std::memory_order_acquire),
        std::memory_order_release);
    Log.noticeln("drawnotevenodd = %B",
                 vic->drawnotevenodd.load(std::memory_order_acquire));
    break;
  case SETJOYSTICKMODE:
    cpu->joystickmode++;
    if (cpu->joystickmode > CPUC64::JOYSTICKM::JOYSTICKP2) {
      cpu->joystickmode = CPUC64::JOYSTICKM::NOJOYSTICK;
    }
    Log.noticeln("joystickmode = %x", cpu->joystickmode);
    break;
  case SETKBJOYSTICKMODE:
    cpu->kbjoystickmode++;
    if (cpu->kbjoystickmode > CPUC64::JOYSTICKM::JOYSTICKP2) {
      cpu->kbjoystickmode = CPUC64::JOYSTICKM::NOJOYSTICK;
    }
    Log.noticeln("kbjoystickmode = %x", cpu->kbjoystickmode);
    break;
  }
  ExternalCmds::hostcmdcode = NOHOSTCMD;
}
