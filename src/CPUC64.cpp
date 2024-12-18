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
#include "C64Emu.h"
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
  } else if (ciaidx == 0x08) {
    uint8_t val;
    if (cia.isTODFreezed) {
      val = cia.ciareg[ciaidx];
    } else {
      val = cia.latchrundc08.load(std::memory_order_acquire);
    }
    cia.isTODFreezed = false;
    return val;
  } else if (ciaidx == 0x09) {
    if (cia.isTODFreezed) {
      return cia.ciareg[ciaidx];
    } else {
      return cia.latchrundc09.load(std::memory_order_acquire);
    }
  } else if (ciaidx == 0x0a) {
    if (cia.isTODFreezed) {
      return cia.ciareg[ciaidx];
    } else {
      return cia.latchrundc0a.load(std::memory_order_acquire);
    }
  } else if (ciaidx == 0x0b) {
    cia.isTODFreezed = true;
    cia.ciareg[0x08] = cia.latchrundc08.load(std::memory_order_acquire);
    cia.ciareg[0x09] = cia.latchrundc09.load(std::memory_order_acquire);
    cia.ciareg[0x0a] = cia.latchrundc0a.load(std::memory_order_acquire);
    cia.ciareg[0x0b] = cia.latchrundc0b.load(std::memory_order_acquire);
    return cia.ciareg[ciaidx];
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
      } else if (vicidx == 0x11) {
        uint8_t raster8 = (vic->rasterline >= 256) ? 0x80 : 0;
        return vic->vicreg[0x11] & 0x7f | raster8;
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
        uint8_t mask = (cia1.ciareg[0x00] & 0xc0) | 0x3f;
        if (cia1.ciareg[0x02] & 0xc0) {
          mask = (cia1.ciareg[0x00] & 0xc0) | 0x3f;
        } else {
          mask = 0xff;
        }
        if (joystickmode == 2) {
          return joystick.getValue() & mask;
        } else if (kbjoystickmode == 2) {
          return c64emu->blekb.getKBJoyValue(true) & mask;
        } else {
          return cia1.ciareg[0x00] & mask;
        }
      } else if (ciaidx == 0x01) {
        if (joystickmode == 2) {
          // special case: handle fire2 button -> space key
          if ((cia1.ciareg[0x00] == 0x7f) && joystick.getFire2()) {
            return 0xef;
          }
        }
        if (joystickmode == 1) {
          // real joystick, but still check for keyboard input
          uint8_t ret =
              c64emu->blekb.getdc01(cia1.ciareg[0x00] & cia1.ciareg[0x02]);
          if (ret == 0xff) {
            // no key pressed -> return joystick value (of real joystick)
            ret = joystick.getValue();
          }
          return ret;
        } else if (kbjoystickmode == 1) {
          // keyboard joystick, but still check for keyboard input
          uint8_t ret =
              c64emu->blekb.getdc01(cia1.ciareg[0x00] & cia1.ciareg[0x02]);
          if (ret == 0xff) {
            // no key pressed -> return joystick value (of keyboard joystick)
            ret = c64emu->blekb.getKBJoyValue(false);
          }
          return ret;
        } else {
          // keyboard
          return c64emu->blekb.getdc01(cia1.ciareg[0x00] & cia1.ciareg[0x02]);
        }
      }
      return getCommonCIAReg(cia1, ciaidx);
    }
    // ** CIA 2 **
    else if (addr <= 0xddff) {
      uint8_t ciaidx = (addr - 0xdd00) % 0x10;
      if (ciaidx == 0x0d) {
        nmiAck = true;
      }
      return getCommonCIAReg(cia2, ciaidx);
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

inline void setCommonCIAReg(CIA &cia, uint8_t ciaidx, uint8_t val) {
  if (ciaidx == 0x04) {
    cia.latchdc04 = val;
  } else if (ciaidx == 0x05) {
    cia.latchdc05 = val;
    // timerA stopped? if yes, write timerA
    if (!(cia.ciareg[0x0e] & 1)) {
      cia.timerA = (cia.latchdc05 << 8) + cia.latchdc04;
    }
  } else if (ciaidx == 0x06) {
    cia.latchdc06 = val;
  } else if (ciaidx == 0x07) {
    cia.latchdc07 = val;
    // timerB stopped? if yes, write timerB
    if (!(cia.ciareg[0x0f] & 1)) {
      cia.timerB = (cia.latchdc07 << 8) + cia.latchdc06;
    }
  } else if (ciaidx == 0x08) {
    if (cia.ciareg[0x0f] & 128) {
      cia.latchalarmdc08.store(val, std::memory_order_release);
    } else {
      cia.ciareg[0x08] = val;
      cia.latchrundc08.store(val, std::memory_order_release);
      cia.latchrundc09.store(cia.ciareg[0x09], std::memory_order_release);
      cia.latchrundc0a.store(cia.ciareg[0x0a], std::memory_order_release);
      cia.latchrundc0b.store(cia.ciareg[0x0b], std::memory_order_release);
    }
    cia.isTODRunning.store(true, std::memory_order_release);
  } else if (ciaidx == 0x09) {
    if (cia.ciareg[0x0f] & 128) {
      cia.latchalarmdc09.store(val, std::memory_order_release);
    } else {
      cia.ciareg[0x09] = val;
    }
  } else if (ciaidx == 0x0a) {
    if (cia.ciareg[0x0f] & 128) {
      cia.latchalarmdc0a.store(val, std::memory_order_release);
    } else {
      cia.ciareg[0x0a] = val;
    }
  } else if (ciaidx == 0x0b) {
    cia.isTODRunning.store(false, std::memory_order_release);
    if (cia.ciareg[0x0f] & 128) {
      cia.latchalarmdc0b.store(val, std::memory_order_release);
    } else {
      cia.ciareg[0x0b] = val;
    }
  } else if (ciaidx == 0x0c) {
    if (cia.serbitnr == 0) {
      cia.serbitnr = 8;
    } else {
      cia.serbitnrnext = 8;
    }
    cia.ciareg[ciaidx] = val;
  } else if (ciaidx == 0x0d) {
    if (val & 0x80) {
      cia.ciareg[ciaidx] |= val;
    } else {
      cia.ciareg[ciaidx] &= ~(val | 0x80);
    }
  } else if (ciaidx == 0x0e) {
    cia.ciareg[ciaidx] = val;
    if (val & 0x10) {
      cia.timerA = (cia.latchdc05 << 8) + cia.latchdc04;
    }
  } else if (ciaidx == 0x0f) {
    cia.ciareg[ciaidx] = val;
    if (val & 0x10) {
      cia.timerB = (cia.latchdc07 << 8) + cia.latchdc06;
    }
  } else {
    cia.ciareg[ciaidx] = val;
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
        // clear given bits
        /*
        // does not work for all games, e.g. bubble bobble
        uint8_t act = vic->vicreg[vicidx];
        act &= ~val;
        if ((act & 7) == 0) {
          act &= ~0x80;
        }
        vic->vicreg[vicidx] = val;
        */
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
      if (ciaidx == 0x00) {
        cia1.ciareg[ciaidx] = val;
      } else {
        setCommonCIAReg(cia1, ciaidx, val);
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
        cia2.ciareg[ciaidx] = 0x94 | bank;
        // adapt VIC base addresses
        adaptVICBaseAddrs(true);
      } else {
        setCommonCIAReg(cia2, ciaidx, val);
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

void CPUC64::cmd6502halt() {
  cpuhalted = true;
  ESP_LOGE(TAG, "illegal code, cpu halted, pc = %x", pc - 1);
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

void CPUC64::checkciatimers(uint8_t cycles) {
  // CIA 1 TOD alarm
  cia1.checkAlarm();
  // check for CIA 1 TOD alarm interrupt
  if (((cia1.latchdc0d & 0x84) == 0x84) && (!iflag)) {
    setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false, true);
  }
  // CIA 1 Timer A
  cia1.checkTimerA(cycles);
  // check for CIA 1 Timer A interrupt
  if (((cia1.latchdc0d & 0x81) == 0x81) && (!iflag)) {
    setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false, true);
  }
  // CIA 1 Timer B
  cia1.checkTimerB(cycles);
  // check for CIA 1 Timer B interrupt
  if (((cia1.latchdc0d & 0x82) == 0x82) && (!iflag)) {
    setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false, true);
  }
  // check for CIA 1 SDR interrupt
  if (((cia1.latchdc0d & 0x88) == 0x88) && (!iflag)) {
    setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false, true);
  }
  if (!deactivateCIA2) {
    // CIA 2 TOD alarm
    cia2.checkAlarm();
    // check for CIA 2 TOD alarm interrupt
    if (((cia2.latchdc0d & 0x84) == 0x84) && nmiAck) {
      nmiAck = false;
      setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false, false);
    }
    // CIA 2 Timer A
    cia2.checkTimerA(cycles);
    // check for CIA 2 Timer A interrupt
    if (((cia2.latchdc0d & 0x81) == 0x81) && nmiAck) {
      nmiAck = false;
      setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false, false);
    }
    // CIA 2 Timer B
    cia2.checkTimerB(cycles);
    // check for CIA 2 Timer B interrupt
    if (((cia2.latchdc0d & 0x82) == 0x82) && nmiAck) {
      nmiAck = false;
      setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false, false);
    }
    // check for CIA 2 SDR interrupt
    if (((cia2.latchdc0d & 0x88) == 0x88) && nmiAck) {
      nmiAck = false;
      setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false, false);
    }
  }
}

void CPUC64::logDebugInfo() {
  if (debug &&
      (debuggingstarted || (debugstartaddr == 0) || (pc == debugstartaddr))) {
    debuggingstarted = true;
    // debug (use LOGE because LOGI doesn't work here...)
    ESP_LOGE(
        TAG,
        "pc: %2x, cmd: %s, a: %x, sr: %x, d012: %x, d011: %x, timerA(2): %x",
        pc, cmdName[getMem(pc)], a, sr, getMem(0xd012), getMem(0xd011),
        cia2.timerA);
  }
}

void CPUC64::run() {
  // pc *must* be set externally!
  cpuhalted = false;
  debug = false;
  debugstartaddr = 0;
  debuggingstarted = false;
  numofcycles = 0;
  uint8_t badlinecycles = 0;
  while (true) {
    if (cpuhalted) {
      continue;
    }

    // prepare next rasterline
    badlinecycles = vic->nextRasterline();
    if (vic->screenblank) {
      badlinecycles = 0;
    }
    // raster line interrupt?
    if ((vic->vicreg[0x19] & 0x81) && (vic->vicreg[0x1a] & 1) && (!iflag)) {
      setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false, true);
    }
    // execute CPU cycles
    // (4 = average number of cycles for an instruction)
    numofcycles = 0;
    uint8_t numofcyclestoexe = 63 - (4 / 2) - badlinecycles;
    uint8_t i = 0;
    while (numofcycles < numofcyclestoexe) {
      if (cpuhalted) {
        break;
      }
      logDebugInfo();
      execute(getMem(pc++));
    }
    // cia timers
    checkciatimers(numofcycles);
    // draw rasterline
    vic->drawRasterline();
    // sprite collision interrupt?
    if ((vic->vicreg[0x19] & 0x86) && (vic->vicreg[0x1a] & 6) && (!iflag)) {
      setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false, true);
    }
    if (restorenmi && nmiAck) {
      nmiAck = false;
      restorenmi = false;
      setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false, false);
    }
    // throttle 6502 CPU
    numofcyclespersecond += numofcycles;
    measuredcycles.fetch_add(numofcycles, std::memory_order_release);
    uint16_t adjustcyclestmp = adjustcycles.load(std::memory_order_acquire);
    if (adjustcyclestmp > 0) {
      esp_rom_delay_us(adjustcyclestmp);
      adjustcycles.store(0, std::memory_order_release);
    }
  }
}

void CPUC64::initMemAndRegs() {
  ESP_LOGI(TAG, "CPUC64::initMemAndRegs");
  setMem(0, 0x2f);
  setMem(1, 0x37);
  setMem(0x8004, 0);
  sp = 0xFF;
  iflag = true;
  dflag = false;
  bflag = false;
  nmiAck = true;
  restorenmi = false;
  uint16_t addr = 0xfffc - 0xe000;
  pc = kernal_rom[addr] + (kernal_rom[addr + 1] << 8);
}

void CPUC64::init(uint8_t *ram, uint8_t *charrom, VIC *vic, C64Emu *c64emu) {
  ESP_LOGI(TAG, "CPUC64::init");
  this->ram = ram;
  this->charrom = charrom;
  this->vic = vic;
  this->c64emu = c64emu;
  measuredcycles.store(0, std::memory_order_release);
  adjustcycles.store(0, std::memory_order_release);
  joystickmode = 0;
  kbjoystickmode = 0;
  deactivateCIA2 = true;
  numofcycles = 0;
  numofcyclespersecond = 0;
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

void CPUC64::setKeycodes(uint8_t keycode1, uint8_t keycode2) {
  c64emu->blekb.setKbcodes(keycode1, keycode2);
}
