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
#include "CBEServiceLocator.h"
#include "JoystickInitializationException.h"
#include "roms/basic.h"
#include "roms/kernal.h"
#include <esp_log.h>
#include <esp_random.h>

static const char *TAG = "CPUC64";

inline uint8_t getCommonCIAReg(CIA &cia, uint8_t ciaidx) {
  if (ciaidx == 0x04) {
    return cia.timerA & 0xff;
  } else if (ciaidx == 0x05) {
    return (cia.timerA >> 8) & 0xff;
  } else if (ciaidx == 0x06) {
    return cia.timerB & 0xff;
  } else if (ciaidx == 0x07) {
    return (cia.timerB >> 8) & 0xff;
  } else if (ciaidx == 0x0d) {
    uint8_t val = cia.latchdc0d;
    cia.latchdc0d = 0;
    return val;
  } else {
    return cia.ciareg[ciaidx];
  }
}

uint8_t CPUC64::getMem(uint16_t addr) {
  if ((!bankARAM) && ((addr >= 0xa000) && (addr <= 0xbfff))) {
    //    basic rom
    return basic_rom[addr - 0xa000];
  } else if ((!bankERAM) && (addr >= 0xe000)) {
    // kernal rom
    return kernal_rom[addr - 0xe000];
  } else if (bankDIO && (addr >= 0xd000) && (addr <= 0xdfff)) {
    // ** VIC **
    if (addr <= 0xd3ff) {
      uint8_t vicidx = (addr - 0xd000) % 0x40;
      if ((vicidx == 0x1e) || (vicidx == 0x1f)) {
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
      if (ciaidx == 0x00) {
        if (joystickmode == 2) {
          // real joystick
          return joystick.getValue(true) | (cia1.ciareg[0x00] & 0x80);
        } else if (kbjoystickmode == 2) {
          // keyboard joystick
          return CBEServiceLocator::getBLEKB()->getKBJoyValue(true) |
                 (cia1.ciareg[0x00] & 0x80);
        } else {
          return cia1.ciareg[0x00];
        }
      } else if (ciaidx == 0x01) {
        if (joystickmode == 1) {
          // real joystick, but still check for keyboard input
          uint8_t pressedkey =
              CBEServiceLocator::getBLEKB()->decode(cia1.ciareg[0x00]);
          if (pressedkey == 0xff) {
            // no key pressed -> return joystick value (of real joystick)
            return joystick.getValue(false);
          }
          return pressedkey;
        } else if (kbjoystickmode == 1) {
          // keyboard joystick, but still check for keyboard input
          uint8_t pressedkey =
              CBEServiceLocator::getBLEKB()->decode(cia1.ciareg[0x00]);
          if (pressedkey == 0xff) {
            // no key pressed -> return joystick value (of keyboard joystick)
            return CBEServiceLocator::getBLEKB()->getKBJoyValue(false);
          }
          return pressedkey;
        } else {
          // keyboard
          return CBEServiceLocator::getBLEKB()->decode(cia1.ciareg[0x00]);
        }
      } else {
        return getCommonCIAReg(cia1, ciaidx);
      }
    }
    // ** CIA 2 **
    else if (addr <= 0xddff) {
      return getCommonCIAReg(cia2, (addr - 0xdd00) % 0x10);
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
        vic->vicreg[vicidx] = val & 0x7f;
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
        cia1.latchdc04 = val;
      } else if (ciaidx == 0x05) {
        cia1.latchdc05 = val;
        // timerA stopped? if yes, write timerA
        if (!(cia1.ciareg[0x0e] & 1)) {
          cia1.timerA = (cia1.latchdc05 << 8) + cia1.latchdc04;
        }
      } else if (ciaidx == 0x06) {
        cia1.latchdc06 = val;
      } else if (ciaidx == 0x07) {
        cia1.latchdc07 = val;
        // timerB stopped? if yes, write timerB
        if (!(cia1.ciareg[0x0f] & 1)) {
          cia1.timerB = (cia1.latchdc07 << 8) + cia1.latchdc06;
        }
      } else if (ciaidx == 0x0d) {
        if (val & 0x80) {
          cia1.ciareg[ciaidx] |= val;
        } else {
          cia1.ciareg[ciaidx] &= ~val;
        }
      } else if (ciaidx == 0x0e) {
        cia1.ciareg[ciaidx] = val;
        if (val & 0x10) {
          cia1.timerA = (cia1.latchdc05 << 8) + cia1.latchdc04;
        }
        if (val & 0x08) {
          cia1.reloadA = false;
        } else {
          cia1.reloadA = true;
        }
      } else if (ciaidx == 0x0f) {
        cia1.ciareg[ciaidx] = val;
        if (val & 0x10) {
          cia1.timerB = (cia1.latchdc07 << 8) + cia1.latchdc06;
        }
        if (val & 0x08) {
          cia1.reloadB = false;
        } else {
          cia1.reloadB = true;
        }
      } else {
        cia1.ciareg[ciaidx] = val;
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
        cia2.ciareg[ciaidx] = val;
        // adapt VIC base addresses
        adaptVICBaseAddrs(true);
      } else if (ciaidx == 0x04) {
        cia2.latchdc04 = val;
      } else if (ciaidx == 0x05) {
        cia2.latchdc05 = val;
        // timerA stopped? if yes, write timerA
        if (!(cia2.ciareg[0x0e] & 1)) {
          cia2.timerA = (cia2.latchdc05 << 8) + cia2.latchdc04;
        }
      } else if (ciaidx == 0x06) {
        cia2.latchdc06 = val;
      } else if (ciaidx == 0x07) {
        cia2.latchdc07 = val;
        // timerB stopped? if yes, write timerB
        if (!(cia2.ciareg[0x0f] & 1)) {
          cia2.timerB = (cia2.latchdc07 << 8) + cia2.latchdc06;
        }
      } else if (ciaidx == 0x0d) {
        if (val & 0x80) {
          cia2.ciareg[ciaidx] |= val;
        } else {
          cia2.ciareg[ciaidx] &= ~val;
        }
      } else if (ciaidx == 0x0e) {
        cia2.ciareg[ciaidx] = val;
        if (val & 0x10) {
          cia2.timerA = (cia2.latchdc05 << 8) + cia2.latchdc04;
        }
        if (val & 0x08) {
          cia2.reloadA = false;
        } else {
          cia2.reloadA = true;
        }
      } else if (ciaidx == 0x0f) {
        cia2.ciareg[ciaidx] = val;
        if (val & 0x10) {
          cia2.timerB = (cia2.latchdc07 << 8) + cia2.latchdc06;
        }
        if (val & 0x08) {
          cia2.reloadB = false;
        } else {
          cia2.reloadB = true;
        }
      } else {
        cia2.ciareg[ciaidx] = val;
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
  ESP_LOGI(TAG, "illegal code, cpu halted, pc = %x", pc - 1);
}

/*
void CPUC64::cmd6502nop1a() {
  ESP_LOGI(TAG, "nop: %d", micros());
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
  numofcycles = 0;
  uint8_t badlinecycles = 0;
  while (true) {
    if (cpuhalted) {
      continue;
    }
    execute(getMem(pc++));
    if (numofcycles >=
        63 - (4 / 2) -
            badlinecycles) { // 4 = average number of cycles for an instruction
      numofcyclespersecond += numofcycles;
      badlinecycles = vic->nextRasterline();
      // raster line interrupt?
      if ((vic->vicreg[0x19] & 0x80) && (vic->vicreg[0x1a] & 1) && (!iflag)) {
        setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false);
        for (uint8_t i = 0; i < 6; i++) {
          execute(getMem(pc++));
        }
      }
      vic->drawRasterline();
      // sprite collision interrupt?
      if ((vic->vicreg[0x19] & 0x80) && (vic->vicreg[0x1a] & 6) && (!iflag)) {
        setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false);
      }
      // CIA 1 Timer A
      if (cia1.checkTimerA(numofcycles)) {
        if (!iflag) {
          setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false);
        }
      }
      // CIA 1 Timer B
      if (cia1.checkTimerB(numofcycles)) {
        if (!iflag) {
          setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false);
        }
      }
      // CIA 2 Timer A
      if (cia2.checkTimerA(numofcycles)) {
        setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false);
      }
      // CIA 2 Timer B
      if (cia2.checkTimerB(numofcycles)) {
        setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false);
      }
      // throttle 6502 CPU
      measuredcycles.fetch_add(numofcycles, std::memory_order_release);
      uint16_t adjustcyclestmp = adjustcycles.load(std::memory_order_acquire);
      if (adjustcyclestmp > 0) {
        ets_delay_us(adjustcyclestmp);
        adjustcycles.store(0, std::memory_order_release);
      }
      // reset numofcycles
      numofcycles = 0;
    }
  }
}

void CPUC64::initMemAndRegs() {
  setMem(0, 0x2f);
  setMem(1, 0x37);
  sp = 0xFF;
  iflag = true;
  dflag = false;
  bflag = false;
  uint16_t addr = 0xfffc - 0xe000;
  pc = kernal_rom[addr] + (kernal_rom[addr + 1] << 8);
}

void CPUC64::init(uint8_t *ram, uint8_t *charrom, VIC *vic) {
  this->ram = ram;
  this->charrom = charrom;
  this->vic = vic;
  measuredcycles.store(0, std::memory_order_release);
  adjustcycles.store(0, std::memory_order_release);
  joystickmode = 0;
  kbjoystickmode = 0;
  refreshframecolor = true;
  numofcycles = 0;
  numofcyclespersecond = 0;
  adjustcycles = 0;
  try {
    joystick.init();
  } catch (const JoystickInitializationException &e) {
    ESP_LOGI(TAG, "error in init. of joystick: %s - continue anyway", e.what());
  }
  initMemAndRegs();
}

void CPUC64::setPC(uint16_t newPC) {
  std::lock_guard<std::mutex> lock(pcMutex);
  pc = newPC;
}

void CPUC64::exeSubroutine(uint16_t addr, uint8_t rega, uint8_t regx,
                           uint8_t regy) {
  bool tcflag = cflag;
  bool tzflag = zflag;
  bool tdflag = dflag;
  bool tbflag = bflag;
  bool tvflag = vflag;
  bool tnflag = nflag;
  bool tiflag = iflag;
  uint8_t ta = a;
  uint8_t tx = x;
  uint8_t ty = y;
  uint8_t tsp = sp;
  uint8_t tsr = sr;
  uint16_t tpc = pc;
  iflag = true;
  dflag = false;
  bflag = false;
  a = rega;
  x = regx;
  y = regy;
  ram[0x033c] = 0x20; // jsr
  ram[0x033d] = addr & 0xff;
  ram[0x033e] = addr >> 8;
  pc = 0x033c;
  while (true) {
    uint8_t nextopc = getMem(pc++);
    // ESP_LOGI(TAG, "pc = %x, opc = %x, a = %x, x = %x, y = %x", pc-1, nextopc,
    // a, x, y);
    execute(nextopc);
    if ((sp == tsp) && (nextopc == 0x60)) { // rts
      break;
    }
  }
  cflag = tcflag;
  zflag = tzflag;
  dflag = tdflag;
  bflag = tbflag;
  vflag = tvflag;
  nflag = tnflag;
  iflag = tiflag;
  a = ta;
  x = tx;
  y = ty;
  sp = tsp;
  sr = tsr;
  pc = tpc;
}
