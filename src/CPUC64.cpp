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
#include "CPUC64.h"
#include "Config.h"
#include "ExternalCmds.h"
#include "JoystickInitializationException.h"
#include "OSUtils.h"
#include "roms/basic.h"
#include "roms/kernal.h"

static const uint8_t NUMCIACHECKS = 2;

static const char *TAG = "CPUC64";

// read dc00 / dc01:
// from
// https://retrocomputing.stackexchange.com/questions/6421/reading-both-keyboard-and-joystick-with-non-kernal-code-on-c64
// There may be multiple connections to ground, making this a "wired-OR" system:
// any one of these connections, if made to ground, will force the line low
// regardless of how any other connection is set. In other words, it's not
// possible for the line to be high if anything on that line wants to bring it
// low.
// => use "(cia1.ciareg[0x00] | ~ddra) & input;" instead of
// "(cia1.ciareg[0x00] & ddra) | (input & ~ddra);"

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
        return (vic->vicreg[0x11] & 0x7f) | raster8;
      } else {
        return vic->vicreg[vicidx];
      }
    }
    // ** SID **
    else if (addr <= 0xd7ff) {
      uint8_t sididx = (addr - 0xd400) % 0x20;
      if (sididx == 0x1b) {
        return OSUtils::getRandomByte();
      } else if (sididx == 0x1c) {
        return static_cast<uint8_t>(sid.v3envs[vic->rasterline * 881 / 311] *
                                    255.0f);
      } else {
        return sid.sidreg[sididx];
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
        uint8_t ddra = cia1.ciareg[0x02];
        uint8_t input = 0xff;
        if (joystickmode == 2) {
          // real joystick, but still check for keyboard input
          input =
              configKeyboard.keyboardDriver->getDC01(cia1.ciareg[0x01], true);
          if (input == 0xff) {
            // no key pressed -> return joystick value (of real joystick)
            input = joystick.getValue();
          }
        } else if (kbjoystickmode == 2) {
          // keyboard joystick, but still check for keyboard input
          input =
              configKeyboard.keyboardDriver->getDC01(cia1.ciareg[0x01], true);
          if (input == 0xff) {
            // no key pressed -> return joystick value (of keyboard joystick)
            input = configKeyboard.keyboardDriver->getKBJoyValue(true);
          }
        } else {
          // keyboard
          input =
              configKeyboard.keyboardDriver->getDC01(cia1.ciareg[0x01], true);
        }
        return (cia1.ciareg[0x00] | ~ddra) & input;
      } else if (ciaidx == 0x01) {
        uint8_t ddrb = cia1.ciareg[0x03];
        uint8_t input = 0xff;
        if (joystickmode == 2) {
          // special case: handle fire2 button -> space key
          if ((cia1.ciareg[0x00] == 0x7f) && joystick.getFire2()) {
            return 0xef;
          }
        }
        if (joystickmode == 1) {
          // real joystick, but still check for keyboard input
          input =
              configKeyboard.keyboardDriver->getDC01(cia1.ciareg[0x00], false);
          if (input == 0xff) {
            // no key pressed -> return joystick value (of real joystick)
            input = joystick.getValue();
          }
        } else if (kbjoystickmode == 1) {
          // keyboard joystick, but still check for keyboard input
          input =
              configKeyboard.keyboardDriver->getDC01(cia1.ciareg[0x00], false);
          if (input == 0xff) {
            // no key pressed -> return joystick value (of keyboard joystick)
            input = configKeyboard.keyboardDriver->getKBJoyValue(false);
          }
        } else {
          // keyboard
          input =
              configKeyboard.keyboardDriver->getDC01(cia1.ciareg[0x00], false);
        }
        return (cia1.ciareg[0x01] | ~ddrb) & input;
      }
      return cia1.getCommonCIAReg(ciaidx);
    }
    // ** CIA 2 **
    else if (addr <= 0xddff) {
      uint8_t ciaidx = (addr - 0xdd00) % 0x10;
      if (ciaidx == 0x00) {
        uint8_t ddra = cia2.ciareg[0x02];
        return cia2.ciareg[0x00] | ~ddra;
      } else if (ciaidx == 0x01) {
        uint8_t ddrb = cia2.ciareg[0x03];
        return cia2.ciareg[0x01] | ~ddrb;
      } else if (ciaidx == 0x0d) {
        nmiAck = true;
      }
      return cia2.getCommonCIAReg(ciaidx);
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
        vic->badlinecond0 = false;
        if ((vic->rasterline == 0x30) && (val & 0x10)) {
          vic->badlinecond0 = true;
        }
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
      uint8_t sididx = (addr - 0xd400) % 0x20;
      sid.sidreg[sididx] = val;
      if (sididx <= 0x14) {
        uint8_t voice = sididx / 7;
        int regInVoice = sididx % 7;
        switch (regInVoice) {
        case 0:
        case 1:
          sid.sidVoice[voice].updVarFrequency(sid.sidreg[voice * 7] |
                                              (sid.sidreg[1 + voice * 7] << 8));
          break;
        case 2:
        case 3:
          sid.sidVoice[voice].updVarPulseWidth(
              sid.sidreg[2 + voice * 7] | (sid.sidreg[3 + voice * 7] << 8));
          break;
        case 4:
          sid.sidVoice[voice].updVarControl(sid.sidreg[sididx]);
          break;
        case 5:
          sid.sidVoice[voice].updVarEnvelopeAD(sid.sidreg[sididx]);
          break;
        case 6:
          sid.sidVoice[voice].updVarEnvelopeSR(sid.sidreg[sididx]);
          break;
        }
      } else if (sididx == 0x18) {
        sid.c64Volume = (float)(val & 0x0f) / 15.0;
      }
    }
    // ** Colorram **
    else if (addr <= 0xdbff) {
      vic->colormap[addr - 0xd800] = val;
    }
    // ** CIA 1 **
    else if (addr <= 0xdcff) {
      uint8_t ciaidx = (addr - 0xdc00) % 0x10;
      if (ciaidx == 0x00) {
        uint8_t ddra = cia1.ciareg[0x02];
        cia1.ciareg[ciaidx] = (cia1.ciareg[ciaidx] & ~ddra) | (val & ddra);
      } else if (ciaidx == 0x01) {
        uint8_t ddrb = cia1.ciareg[0x03];
        cia1.ciareg[ciaidx] = (cia1.ciareg[ciaidx] & ~ddrb) | (val & ddrb);
      } else {
        cia1.setCommonCIAReg(ciaidx, val);
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
        cia2.setCommonCIAReg(ciaidx, val);
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
  OSUtils::log(LOG_ERROR, TAG, "illegal code, cpu halted, pc = %x", pc - 1);
}

/*
void CPUC64::cmd6502nop1a() {
  OSUtils::log(LOG_INFO, TAG, "nop: %d", micros());
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
    setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false);
  }
  // CIA 1 Timer A
  cia1.checkTimerA(cycles);
  // check for CIA 1 Timer A interrupt
  if (((cia1.latchdc0d & 0x81) == 0x81) && (!iflag)) {
    setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false);
  }
  // CIA 1 Timer B
  cia1.checkTimerB(cycles);
  // check for CIA 1 Timer B interrupt
  if (((cia1.latchdc0d & 0x82) == 0x82) && (!iflag)) {
    setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false);
  }
  // check for CIA 1 SDR interrupt
  if (((cia1.latchdc0d & 0x88) == 0x88) && (!iflag)) {
    setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false);
  }
  // CIA 2 TOD alarm
  cia2.checkAlarm();
  // check for CIA 2 TOD alarm interrupt
  if (((cia2.latchdc0d & 0x84) == 0x84) && nmiAck) {
    nmiAck = false;
    setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false);
  }
  // CIA 2 Timer A
  cia2.checkTimerA(cycles);
  // check for CIA 2 Timer A interrupt
  if (((cia2.latchdc0d & 0x81) == 0x81) && nmiAck) {
    nmiAck = false;
    setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false);
  }
  // CIA 2 Timer B
  cia2.checkTimerB(cycles);
  // check for CIA 2 Timer B interrupt
  if (((cia2.latchdc0d & 0x82) == 0x82) && nmiAck) {
    nmiAck = false;
    setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false);
  }
  // check for CIA 2 SDR interrupt
  if (((cia2.latchdc0d & 0x88) == 0x88) && nmiAck) {
    nmiAck = false;
    setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false);
  }
}

void CPUC64::logDebugInfo() {
  if (debug &&
      (debuggingstarted || (debugstartaddr == 0) || (pc == debugstartaddr))) {
    debuggingstarted = true;
    OSUtils::log(LOG_INFO, TAG,
                 "pc: %2x, cmd: %s, a: %x, x: %x, y: %x, sp: %x, sr: %x", pc,
                 cmdName[getMem(pc)], a, x, y, sp, sr);
  }
}

void CPUC64::vTaskDelayUntilUS(int64_t lastMeasuredTime,
                               uint32_t timeIncrement) {
  int64_t now = OSUtils::getTimeUS();
  int64_t nextWake = lastMeasuredTime + timeIncrement;
  if (now < nextWake) {
    uint32_t pause = (uint32_t)(nextWake - now);
    numofburnedcyclespersecond.fetch_add(pause, std::memory_order_release);
    OSUtils::pauseExecutionUS(pause);
  }
}

void CPUC64::check4extcmd() {
  uint8_t *extcmdbuffer = configKeyboard.keyboardDriver->getExtCmdBuffer();
  if (extcmdbuffer != nullptr) {
    uint8_t type = externalCmds->executeExternalCmd(extcmdbuffer);
    // sync detectreleasekey
    configKeyboard.keyboardDriver->setDetectReleasekey(detectreleasekey);
    // send notification
    uint8_t *data;
    size_t size;
    switch (type) {
    case 1:
      data = reinterpret_cast<uint8_t *>(&(externalCmds->type1notification));
      size = sizeof(externalCmds->type1notification);
      break;
    case 2:
      data = reinterpret_cast<uint8_t *>(&(externalCmds->type2notification));
      size = sizeof(externalCmds->type2notification);
      break;
    case 3:
      data = reinterpret_cast<uint8_t *>(&(externalCmds->type3notification));
      size = sizeof(externalCmds->type3notification);
      break;
    case 4:
      data = reinterpret_cast<uint8_t *>(&(externalCmds->type4notification));
      size = sizeof(externalCmds->type4notification);
      break;
    case 5:
      data = reinterpret_cast<uint8_t *>(&(externalCmds->type5notification));
      size = sizeof(externalCmds->type5notification);
      break;
    default:
      type = 0;
    }
    if (type > 0) {
      configKeyboard.keyboardDriver->sendExtCmdNotification(data, size);
      OSUtils::log(LOG_INFO, TAG, "notification sent");
    }
  }
}

void CPUC64::run() {
  // pc *must* be set externally!
  cpuhalted = false;
  debug = false;
  debugstartaddr = 0;
  debuggingstarted = false;
  detectreleasekey = true;
  numofcycles = 0;
  uint8_t badlinecycles = 0;
  int64_t lastMeasuredTime = OSUtils::getTimeUS();
  while (true) {
    if (cpuhalted) {
      continue;
    }

    // prepare next rasterline
    badlinecycles = vic->nextRasterline();

    if (deactivateTemp) {
      badlinecycles = 0;
    }
    numofcycles = 0;
    int8_t numofcyclestoexe = 63 - badlinecycles;
    if (numofcyclestoexe < 0) {
      numofcyclestoexe = 0;
    }
    uint8_t n = 1;
    if (numofcyclestoexe > NUMCIACHECKS * 20) {
      n = NUMCIACHECKS;
    }
    // raster line interrupt?
    if ((vic->vicreg[0x19] & 0x81) && (vic->vicreg[0x1a] & 1) && (!iflag)) {
      // execute one command to simulate "finish execution of actual command"
      logDebugInfo();
      execute(getMem(pc++));
      // set pc to interrupt vector and start to execute interrupt routine
      setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false);
    }
    // execute CPU cycles and check CIA timers
    uint8_t tmp = numofcyclestoexe / n;
    uint8_t sumtmp = tmp;
    for (uint8_t i = 0; i < n - 1; i++) {
      while (numofcycles < sumtmp) {
        if (cpuhalted) {
          break;
        }
        logDebugInfo();
        execute(getMem(pc++));
      }
      checkciatimers(tmp);
      sumtmp += tmp;
    }
    tmp = numofcycles;
    while (numofcycles < numofcyclestoexe) {
      if (cpuhalted) {
        break;
      }
      logDebugInfo();
      execute(getMem(pc++));
    }
    checkciatimers(numofcycles - tmp);
    // draw rasterline
    vic->drawRasterline();
    // sprite collision interrupt?
    if ((vic->vicreg[0x19] & 0x86) && (vic->vicreg[0x1a] & 6) && (!iflag)) {
      setPCToIntVec(getMem(0xfffe) + (getMem(0xffff) << 8), false);
    }
    // restore key pressed?
    if (restorenmi && nmiAck) {
      nmiAck = false;
      restorenmi = false;
      setPCToIntVec(getMem(0xfffa) + (getMem(0xfffb) << 8), false);
    }
    // check for "external commands" once per frame
    check4extcmd();
    // "throttle"
    numofcyclespersecond.fetch_add(numofcycles, std::memory_order_release);
    uint32_t nominaltime = (vic->rasterline + 1) * 1000000 / 50 / 312;
    vTaskDelayUntilUS(lastMeasuredTime, nominaltime);
    // get start time of frame, sid
    if (vic->rasterline == 311) {
      lastMeasuredTime = OSUtils::getTimeUS();
      sid.playAudio();
    } else if (vic->rasterline == Config::AUDIO_FILLBUFFER_RASTERLINE) {
      sid.fillBuffer();
    }
  }
}

void CPUC64::initMemAndRegs() {
  OSUtils::log(LOG_INFO, TAG, "CPUC64::initMemAndRegs");
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

void CPUC64::init(uint8_t *ram, uint8_t *charrom, VIC *vic) {
  OSUtils::log(LOG_INFO, TAG, "CPUC64::init");
  this->ram = ram;
  this->charrom = charrom;
  this->vic = vic;
  this->externalCmds = new ExternalCmds();
  joystickmode = 0;
  kbjoystickmode = 0;
  deactivateTemp = false;
  numofcycles = 0;
  numofcyclespersecond.store(0, std::memory_order_release);
  numofburnedcyclespersecond.store(0, std::memory_order_release);
  perf.store(false, std::memory_order_release);
  batteryVoltage.store(0, std::memory_order_release);
  poweroff.store(false, std::memory_order_release);

  configKeyboard.keyboardDriver->init();
  try {
    joystick.init();
  } catch (const JoystickInitializationException &e) {
    OSUtils::log(LOG_INFO, TAG,
                 "error in init. of joystick: %s - continue anyway", e.what());
  }
  sid.init();
  initMemAndRegs();
  externalCmds->init(ram, this);
}

void CPUC64::setPC(uint16_t newPC) { pc = newPC; }

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
  configKeyboard.keyboardDriver->setKBcodes(keycode1, keycode2);
}
