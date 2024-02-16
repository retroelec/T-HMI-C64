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
#include "CPUC64.h"

#include <ArduinoLog.h>
#include <esp_random.h>

uint8_t CPUC64::getMem(uint16_t addr) {
  if ((!bankARAM) && ((addr >= 0xa000) && (addr <= 0xbfff))) {
    //    basic rom
    return basicrom[addr - 0xa000];
  } else if ((!bankERAM) && (addr >= 0xe000)) {
    // kernal rom
    return kernalrom[addr - 0xe000];
  } else if (bankDIO && (addr >= 0xd000) && (addr <= 0xdfff)) {
    // ** VIC **
    if (addr <= 0xd3ff) {
      uint8_t vicidx = (addr - 0xd000) % 0x40;
      if (vicidx == 0x11) {
        return vic->vicreg[vicidx] | ((vic->rasterline >> 1) & 0x80);
      } else if (vicidx == 0x12) {
        return vic->rasterline & 0xff;
      } else if ((vicidx == 0x1e) || (vicidx == 0x1f)) {
        uint8_t val = vic->vicreg[vicidx];
        vic->vicreg[vicidx] = 0;
        return val;
      } else {
        return vic->vicreg[vicidx];
      }
    }
    // ** SID resp RNG **
    else if (addr <= 0xd7ff) {
      uint8_t sididx = (addr - 0xd400) % 0x100;
      if (sididx == 0x1b) {
        uint32_t rand = esp_random();
        return (uint8_t)(rand & 0xff);
      } else if (sididx == 0x1c) {
        return 0;
      } else {
        return sidreg[sididx];
      }
    }
    // ** Colorram **
    else if (addr <= 0xdbff) {
      return vic->colormap[addr - 0xd800];
    }
    // ** CIA 1 **
    else if (addr <= 0xdcff) {
      uint8_t ciaidx = (addr - 0xdc00) % 0x10;
      if (ciaidx == 0x04) {
        return cia1->timerA.load(std::memory_order_acquire) & 0xff;
      } else if (ciaidx == 0x05) {
        return (cia1->timerA.load(std::memory_order_acquire) >> 8) & 0xff;
      } else if (ciaidx == 0x06) {
        return cia1->timerB.load(std::memory_order_acquire) & 0xff;
      } else if (ciaidx == 0x07) {
        return (cia1->timerB.load(std::memory_order_acquire) >> 8) & 0xff;
      } else if (ciaidx == 0x0d) {
        uint8_t val = cia1->latchdc0d.load(std::memory_order_acquire);
        cia1->latchdc0d.store(0, std::memory_order_release);
        return val;
      } else if (ciaidx == 0x00) {
        if (joystickmode == JOYSTICKM::JOYSTICKP2) {
          // real joystick
          return Joystick::getValue();
        } else if (kbjoystickmode == JOYSTICKM::JOYSTICKP2) {
          // keyboard emulated joystick (port 2)
          return blekb->kbcode2;
        } else {
          return 0x7f;
        }
      } else if (ciaidx == 0x01) {
        if (joystickmode == JOYSTICKM::JOYSTICKP1) {
          // real joystick, but still check for keyboard input
          uint8_t pressedkey =
              blekb->decode(cia1->ciareg[0x00].load(std::memory_order_acquire));
          if (pressedkey = 0xff) {
            // no key pressed -> return joystick value (of real joystick)
            return Joystick::getValue();
          }
          return pressedkey;
        } else if (kbjoystickmode == JOYSTICKM::JOYSTICKP1) {
          // keyboard emulated joystick (port 1)
          return blekb->kbcode2;
        } else {
          // keyboard
          return blekb->decode(
              cia1->ciareg[0x00].load(std::memory_order_acquire));
        }
      } else {
        return cia1->ciareg[ciaidx].load(std::memory_order_acquire);
      }
    }
    // ** CIA 2 **
    else if (addr <= 0xddff) {
      uint8_t ciaidx = (addr - 0xdd00) % 0x10;
      return cia2->ciareg[ciaidx].load(std::memory_order_acquire);
    }
  } else if ((!bankDRAM) && (addr >= 0xd000) && (addr <= 0xdfff)) {
    // dxxx character rom
    return charrom[addr - 0xd000];
  } else if (addr == 0x0001) {
    return register1;
  }
  // ram
  return ram[addr];
}

void CPUC64::decodeRegister1(uint8_t val) {
  switch (val) {
  case 0:
  case 4:
    bankARAM = true;
    bankDRAM = true;
    bankERAM = true;
    bankDIO = false;
    break;
  case 1:
    bankARAM = true;
    bankDRAM = false;
    bankERAM = true;
    bankDIO = false;
    break;
  case 2:
    bankARAM = true;
    bankDRAM = false;
    bankERAM = false;
    bankDIO = false;
    break;
  case 3:
    bankARAM = false;
    bankDRAM = false;
    bankERAM = false;
    bankDIO = false;
    break;
  case 5:
    bankARAM = true;
    bankDRAM = true;
    bankERAM = true;
    bankDIO = true;
    break;
  case 6:
    bankARAM = true;
    bankDRAM = true;
    bankERAM = false;
    bankDIO = true;
    break;
  case 7:
    bankARAM = false;
    bankDRAM = true;
    bankERAM = false;
    bankDIO = true;
    break;
  }
}

void CPUC64::adaptVICBaseAddrs(bool fromcia) {
  uint8_t val = vic->vicreg[0x18];
  uint16_t val1 = (val & 0xf0) << 6;
  uint16_t vicmem = vic->vicmem;
  // screenmem is used for text mode and bitmap mode
  vic->screenmemstart = vicmem + val1;
  bool bmm = vic->vicreg[0x11] & 32;
  if ((bmm) || fromcia) {
    if ((val & 8) == 0) {
      vic->bitmapstart = vicmem;
    } else {
      vic->bitmapstart = vicmem + 0x2000;
    }
  }
  uint16_t charmemstart;
  if ((!bmm) || fromcia) {
    // charactermem
    val1 = (val & 0x0e) << 10;
    charmemstart = vicmem + val1;
    if ((charmemstart == 0x1800) || (charmemstart == 0x9800)) {
      vic->charset = vic->chrom + 0x0800;
    } else if ((charmemstart == 0x1000) || (charmemstart == 0x9000)) {
      vic->charset = vic->chrom;
    } else {
      vic->charset = ram + charmemstart;
    }
  }
}

void CPUC64::setMem(uint16_t addr, uint8_t val) {
  if (bankDIO && (addr >= 0xd000) && (addr <= 0xdfff)) {
    // ** VIC **
    if (addr <= 0xd3ff) {
      uint8_t vicidx = (addr - 0xd000) % 0x40;
      if (vicidx == 0x11) {
        // only bit 7 of latch register d011 is used
        vic->latchd011 = val;
        vic->vicreg[vicidx] = val & 127;
        adaptVICBaseAddrs(false);
      } else if (vicidx == 0x12) {
        vic->latchd012 = val;
      } else if (vicidx == 0x16) {
        vic->vicreg[vicidx] = val;
        adaptVICBaseAddrs(false);
      } else if (vicidx == 0x18) {
        vic->vicreg[vicidx] = val;
        adaptVICBaseAddrs(false);
      } else if (vicidx == 0x19) {
        // just clear all interrupt bits
        vic->vicreg[vicidx] = 0;
      } else if ((vicidx == 0x1e) || (vicidx == 0x1f)) {
        vic->vicreg[vicidx] = 0;
      } else {
        vic->vicreg[vicidx] = val;
      }
    }
    // ** SID **
    else if (addr <= 0xd7ff) {
      uint8_t sididx = (addr - 0xd400) % 0x100;
      sidreg[sididx] = val;
    }
    // ** Colorram **
    else if (addr <= 0xdbff) {
      vic->colormap[addr - 0xd800] = val;
    }
    // ** CIA 1 **
    else if (addr <= 0xdcff) {
      uint8_t ciaidx = (addr - 0xdc00) % 0x10;
      if (ciaidx == 0x04) {
        cia1->latchdc04.store(val, std::memory_order_release);
      } else if (ciaidx == 0x05) {
        cia1->latchdc05.store(val, std::memory_order_release);
        // timerA stopped? if yes, write timerA
        if (!(cia1->ciareg[0x0e] & 1)) {
          cia1->timerA.store(
              (cia1->latchdc05.load(std::memory_order_acquire) << 8) +
                  cia1->latchdc04.load(std::memory_order_acquire),
              std::memory_order_release);
        }
      } else if (ciaidx == 0x06) {
        cia1->latchdc06.store(val, std::memory_order_release);
      } else if (ciaidx == 0x07) {
        cia1->latchdc07.store(val, std::memory_order_release);
        // timerB stopped? if yes, write timerB
        if (!(cia1->ciareg[0x0f] & 1)) {
          cia1->timerB.store(
              (cia1->latchdc07.load(std::memory_order_acquire) << 8) +
                  cia1->latchdc06.load(std::memory_order_acquire),
              std::memory_order_release);
        }
      } else if (ciaidx == 0x0d) {
        if (val & 0x80) {
          cia1->ciareg[ciaidx].fetch_or(val, std::memory_order_release);
        } else {
          cia1->ciareg[ciaidx].fetch_and(~val, std::memory_order_release);
        }
      } else if (ciaidx == 0x0e) {
        cia1->ciareg[ciaidx].store(val, std::memory_order_release);
        if (val & 0x10) {
          cia1->timerA.store(
              (cia1->latchdc05.load(std::memory_order_acquire) << 8) +
                  cia1->latchdc04.load(std::memory_order_acquire),
              std::memory_order_release);
        }
        if (val & 0x08) {
          cia1->reloadA.store(false, std::memory_order_release);
        } else {
          cia1->reloadA.store(true, std::memory_order_release);
        }
      } else if (ciaidx == 0x0f) {
        cia1->ciareg[ciaidx].store(val, std::memory_order_release);
        if (val & 0x10) {
          cia1->timerB.store(
              (cia1->latchdc07.load(std::memory_order_acquire) << 8) +
                  cia1->latchdc06.load(std::memory_order_acquire),
              std::memory_order_release);
        }
        if (val & 0x08) {
          cia1->reloadB.store(false, std::memory_order_release);
        } else {
          cia1->reloadB.store(true, std::memory_order_release);
        }
      } else {
        cia1->ciareg[ciaidx].store(val, std::memory_order_release);
      }
    }
    // ** CIA 2 **
    else if (addr <= 0xddff) {
      uint8_t ciaidx = (addr - 0xdd00) % 0x10;
      if (ciaidx == 0x00) {
        uint8_t bank = val & 3;
        switch (bank) {
        case 0:
          vic->vicmem = 0xc000;
          break;
        case 1:
          vic->vicmem = 0x8000;
          break;
        case 2:
          vic->vicmem = 0x4000;
          break;
        case 3:
          vic->vicmem = 0x0000;
          break;
        }
        cia2->ciareg[ciaidx] = val;
        // adapt VIC base addresses
        adaptVICBaseAddrs(true);
      } else if (ciaidx == 0x04) {
        cia2->latchdc04.store(val, std::memory_order_release);
      } else if (ciaidx == 0x05) {
        cia2->latchdc05.store(val, std::memory_order_release);
        // timerA stopped? if yes, write timerA
        if (!(cia2->ciareg[0x0e] & 1)) {
          cia2->timerA.store(
              (cia2->latchdc05.load(std::memory_order_acquire) << 8) +
                  cia2->latchdc04.load(std::memory_order_acquire),
              std::memory_order_release);
        }
      } else if (ciaidx == 0x06) {
        cia2->latchdc06.store(val, std::memory_order_release);
      } else if (ciaidx == 0x07) {
        cia2->latchdc07.store(val, std::memory_order_release);
        // timerB stopped? if yes, write timerB
        if (!(cia2->ciareg[0x0f] & 1)) {
          cia2->timerB.store(
              (cia2->latchdc07.load(std::memory_order_acquire) << 8) +
                  cia2->latchdc06.load(std::memory_order_acquire),
              std::memory_order_release);
        }
      } else if (ciaidx == 0x0d) {
        if (val & 0x80) {
          cia2->ciareg[ciaidx].fetch_or(val, std::memory_order_release);
        } else {
          cia2->ciareg[ciaidx].fetch_and(~val, std::memory_order_release);
        }
      } else if (ciaidx == 0x0e) {
        cia2->ciareg[ciaidx].store(val, std::memory_order_release);
        if (val & 0x10) {
          cia2->timerA.store(
              (cia2->latchdc05.load(std::memory_order_acquire) << 8) +
                  cia2->latchdc04.load(std::memory_order_acquire),
              std::memory_order_release);
        }
        if (val & 0x08) {
          cia2->reloadA.store(false, std::memory_order_release);
        } else {
          cia2->reloadA.store(true, std::memory_order_release);
        }
      } else if (ciaidx == 0x0f) {
        cia2->ciareg[ciaidx].store(val, std::memory_order_release);
        if (val & 0x10) {
          cia2->timerB.store(
              (cia2->latchdc07.load(std::memory_order_acquire) << 8) +
                  cia2->latchdc06.load(std::memory_order_acquire),
              std::memory_order_release);
        }
        if (val & 0x08) {
          cia2->reloadB.store(false, std::memory_order_release);
        } else {
          cia2->reloadB.store(true, std::memory_order_release);
        }
      } else {
        cia2->ciareg[ciaidx].store(val, std::memory_order_release);
      }
    }
  }
  // ** register 1 **
  else if (addr == 0x0001) {
    register1 = val;
    decodeRegister1(register1 & 7);
  }
  // ** ram **
  else {
    ram[addr] = val;
  }
}

void CPUC64::cmd6502illegal() {
  cpuhalted = true;
  Log.noticeln("illegal code, cpu halted, pc = %x", pc - 1);
}

/*
void CPUC64::cmd6502nop1a() {
  Log.noticeln("nop: %d", micros());
  numofcycles += 2;
}
*/

uint8_t CPUC64::getA() { return a; }

uint8_t CPUC64::getX() { return x; }

uint8_t CPUC64::getY() { return y; }

uint8_t CPUC64::getSP() { return sp; }

uint8_t CPUC64::getSR() { return sr; }

uint16_t CPUC64::getPC() { return pc; }

void CPUC64::run() {
  // pc *must* be set externally!
  cpuhalted = false;
  sp = 0xFF;
  iflag = true;
  dflag = false;
  bflag = false;
  irq.store(false, std::memory_order_release);
  numofcycles = 0;
  while (true) {
    if (cpuhalted) {
      continue;
    }
    if (irq.load(std::memory_order_acquire)) {
      irq.store(false, std::memory_order_release);
      if (!iflag) {
        setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8));
      }
    }
    execute(getMem(pc++));
    if (numofcycles >= 63) {
      numofcyclespersecond += numofcycles;
      numofcycles = 0;
      bool vicint = vic->nextRasterline();
      if (vicint && (!iflag)) {
        setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8));
        for (uint8_t i = 0; i < 8; i++) {
          execute(getMem(pc++));
        }
      }
      vic->drawRasterline();
    }
    /*
    if (debugcpu) {
      Log.noticeln("pc = %x, a = %x, x = %x, y = %x, sr = %B", pc, a, x, y, sr);
    }
    */
  }
}

void CPUC64::init(uint8_t *ram, uint8_t *basicrom, uint8_t *kernalrom,
                  uint8_t *charrom, VIC *vic, CIA *cia1, CIA *cia2,
                  BLEKB *blekb) {
  this->ram = ram;
  this->basicrom = basicrom;
  this->kernalrom = kernalrom;
  this->charrom = charrom;
  this->vic = vic;
  this->cia1 = cia1;
  this->cia2 = cia2;
  this->blekb = blekb;
  joystickmode = 0;
  kbjoystickmode = 0;
  setMem(0, 0x2f);
  setMem(1, 0x37);
  numofcyclespersecond = 0;
  debugcpu = false;
  uint16_t addr = 0xfffc - 0xe000;
  pc = kernalrom[addr] + (kernalrom[addr + 1] << 8);
}

void CPUC64::setPC(uint16_t newPC) {
  std::atomic<uint16_t> &atomicVar =
      *reinterpret_cast<std::atomic<uint16_t> *>(&pc);
  atomicVar.store(newPC, std::memory_order_release);
  Log.noticeln("setting pc to %x", pc);
}
