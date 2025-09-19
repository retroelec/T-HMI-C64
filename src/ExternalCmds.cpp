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
#include "ExternalCmds.h"

#include "C64Sys.h"
#include "ExtCmd.h"
#include "fs/FSFactory.h"
#include "listactions.h"
#include "loadactions.h"
#include "platform/PlatformManager.h"
#include "saveactions.h"
#include <cstring>

static const char *TAG = "ExternalCmds";

void ExternalCmds::init(uint8_t *ram, C64Sys *cpu) {
  this->ram = ram;
  this->cpu = cpu;
  sendrawkeycodes = false;
  liststartflag = true;
  filesys = FileSys::create();
}

void ExternalCmds::setType1Notification() {
  type1notification.type = 1;
  type1notification.joymode = cpu->joystickmode;
  type1notification.deactivateTemp = cpu->deactivateTemp;
  type1notification.sendrawkeycodes = sendrawkeycodes;
  type1notification.switchdebug = cpu->debug;
  type1notification.switchperf = cpu->perf.load(std::memory_order_acquire);
  type1notification.switchdetectreleasekey = cpu->detectreleasekey;
}

void ExternalCmds::setType2Notification() {
  type2notification.type = 2;
  type2notification.cpuRunning = !cpu->cpuhalted;
  type2notification.pc = cpu->getPC();
  type2notification.a = cpu->getA();
  type2notification.x = cpu->getX();
  type2notification.y = cpu->getY();
  type2notification.sr = cpu->getSR();
  type2notification.d011 = cpu->getMem(0xd011);
  type2notification.d016 = cpu->getMem(0xd016);
  type2notification.d018 = cpu->getMem(0xd018);
  type2notification.d019 = cpu->getMem(0xd019);
  type2notification.d01a = cpu->getMem(0xd01a);
  type2notification.register1 = cpu->getMem(1);
  type2notification.dc0d = cpu->getMem(0xdc0d);
  type2notification.dc0e = cpu->getMem(0xdc0e);
  type2notification.dc0f = cpu->getMem(0xdc0f);
  type2notification.dd0d = cpu->getMem(0xdd0d);
  type2notification.dd0e = cpu->getMem(0xdd0e);
  type2notification.dd0f = cpu->getMem(0xdd0f);
}

void ExternalCmds::setType3Notification(uint16_t addr) {
  type3notification.type = 3;
  for (uint8_t i = 0; i < NOTIFICATIONTYPE3NUMOFBYTES; i++) {
    type3notification.mem[i] = cpu->getMem(addr + i);
  }
}

void ExternalCmds::setType4Notification() { type4notification.type = 4; }

void ExternalCmds::setType5Notification(uint8_t batteryVolLow,
                                        uint8_t batteryVolHi) {
  type5notification.type = 5;
  type5notification.batteryVolLow = batteryVolLow;
  type5notification.batteryVolHi = batteryVolHi;
}

void ExternalCmds::setType6Notification(uint8_t value) {
  type6notification.type = 6;
  type6notification.value = value;
}

void ExternalCmds::setVarTab(uint16_t addr) {
  // set VARTAB
  ram[0x2d] = addr % 256;
  ram[0x2e] = addr / 256;
  // clr
  cpu->setPC(0xa52a);
}

void getFilename(char *path, uint8_t *ram) {
  uint8_t cury = ram[0xd6];
  uint8_t curx = ram[0xd3];
  uint8_t *cursorpos = ram + 0x0400 + cury * 40 + curx;
  cursorpos--; // char may be 160
  while (*cursorpos == 32) {
    cursorpos--;
  }
  while ((*cursorpos != 32) && (cursorpos >= ram + 0x0400)) {
    cursorpos--;
  }
  cursorpos++;
  uint8_t i = 0;
  uint8_t p;
  while (((p = *cursorpos++) != 32) && (p != 160) && (i < 17)) {
    if ((p >= 1) && (p <= 26)) {
      path[i] = p + 96;
    } else if ((p >= 33) && (p <= 63)) {
      path[i] = p;
    }
    i++;
  }
  path[i++] = '.';
  path[i++] = 'p';
  path[i++] = 'r';
  path[i++] = 'g';
  path[i] = '\0';
}

bool ExternalCmds::isBasicInputMode() {
  uint16_t pc = cpu->getPC();
  if ((cpu->getMem(1) & 7) != 7)
    return false;
  uint16_t irqVec = ram[0x0314] | (ram[0x0315] << 8);
  if (irqVec != 0xea31)
    return false;
  if (pc == 0xe5d1 || pc == 0xe5d4 || pc == 0xe5cd || pc == 0xe5cf)
    return true;
  if (pc >= 0xea31 && pc < 0xefff)
    return true;
  return false;
}

void ExternalCmds::dispVolume() {
  uint8_t volume = cpu->sid.getEmuVolume() * 99 / 255;
  cpu->vic.dispOverlayInfo(volume / 10 + '0', volume % 10 + '0');
}

uint8_t ExternalCmds::executeExternalCmd(uint8_t *buffer) {
  ExtCmd cmd = static_cast<ExtCmd>(buffer[0]);
  switch (cmd) {
  case ExtCmd::NOEXTCMD:
    return 0;
  case ExtCmd::LOAD: {
    if (!isBasicInputMode()) {
      return 0;
    }
    PlatformManager::getInstance().log(LOG_INFO, TAG, "load from file system");
    cpu->cpuhalted = true;
    bool fileloaded = false;
    bool error = false;
    uint16_t addr;
    if (filesys->init()) {
      char filename[21];
      getFilename(filename, ram);
      addr = filesys->load(filename, ram);
      if (addr == 0) {
        PlatformManager::getInstance().log(LOG_INFO, TAG, "file not found");
      } else {
        setVarTab(addr);
        fileloaded = true;
      }
    } else {
      error = true;
      PlatformManager::getInstance().log(LOG_INFO, TAG,
                                         "error init file system");
    }
    addr = src_loadactions_prg[0] + (src_loadactions_prg[1] << 8);
    memcpy(ram + addr, src_loadactions_prg + 2, src_loadactions_prg_len - 2);
    if (fileloaded) {
      cpu->exeSubroutine(addr, 1, 0, 0);
    } else if (error) {
      cpu->exeSubroutine(addr, 0, 1, 0);
    } else {
      cpu->exeSubroutine(addr, 0, 0, 0);
    }
    cpu->cpuhalted = false;
    return 0;
  }
  case ExtCmd::SAVE: {
    if (!isBasicInputMode()) {
      return 0;
    }
    PlatformManager::getInstance().log(LOG_INFO, TAG, "save to file system");
    cpu->cpuhalted = true;
    bool filesaved = false;
    if (filesys->init()) {
      char filename[21];
      getFilename(filename, ram);
      uint16_t startaddr = ram[43] + ram[44] * 256;
      uint16_t endaddr = ram[45] + ram[46] * 256;
      filesaved = filesys->save(filename, ram, startaddr, endaddr);
      if (!filesaved) {
        PlatformManager::getInstance().log(LOG_INFO, TAG, "error saving file");
      }
    } else {
      PlatformManager::getInstance().log(LOG_INFO, TAG,
                                         "error init file system");
    }
    uint16_t addr = src_saveactions_prg[0] + (src_saveactions_prg[1] << 8);
    memcpy(ram + addr, src_saveactions_prg + 2, src_saveactions_prg_len - 2);
    if (filesaved) {
      cpu->exeSubroutine(addr, 1, 0, 0);
    } else {
      cpu->exeSubroutine(addr, 0, 0, 0);
    }
    cpu->cpuhalted = false;
    return 0;
  }
  case ExtCmd::LIST: {
    if (!isBasicInputMode()) {
      return 0;
    }
    PlatformManager::getInstance().log(LOG_INFO, TAG, "list file system");
    cpu->cpuhalted = true;
    if (filesys->init()) {
      uint16_t addr = src_listactions_prg[0] + (src_listactions_prg[1] << 8);
      memcpy(ram + addr, src_listactions_prg + 2, src_listactions_prg_len - 2);
      if (liststartflag) {
        cpu->exeSubroutine(addr, 0, 1, 0);
      } else {
        cpu->exeSubroutine(addr, 0, 2, 0);
      }
      uint8_t filename[17];
      int cnt = 0;
      while (cnt < 23) {
        bool success = filesys->listnextentry(filename, liststartflag);
        liststartflag = false;
        if (success && (filename[0] != '\0')) {
          // copy filename to c64 ram (0x0342)
          for (uint8_t i = 0; i < 17; i++) {
            ram[0x342 + i] = filename[i];
          }
          // print it
          cpu->exeSubroutine(addr, 0, 0, 0);
        } else {
          if (!success) {
            PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                               "error reading entry");
          }
          liststartflag = true;
          break;
        }
        cnt++;
      }
    } else {
      PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                         "error init file system");
    }
    cpu->cpuhalted = false;
    return 0;
  }
  case ExtCmd::RECEIVEDATA: {
    if (!isBasicInputMode()) {
      return 0;
    }
    PlatformManager::getInstance().log(LOG_INFO, TAG, "enter receivedata");
    cpu->cpuhalted = true;
    // simple "protocol":
    // - byte 0: cmd (as usual)
    // - byte 1: cmd detail: first block (1), next block (0), last block (2)
    // - byte 2: cmd flag (as usual)
    // - first block: byte 3 - 4: start address, 5 - 252: data
    // - next block: byte 3 - 252: data
    // - last block: byte 3: length of last block, byte 4 - (length+4-1): data
    uint8_t cmddetail = buffer[1];
    if (cmddetail == 0) {
      // next block
      PlatformManager::getInstance().log(LOG_INFO, TAG, "next block: %x",
                                         actaddrreceivecmd);
      for (uint8_t i = 3; i < 253; i++) {
        ram[actaddrreceivecmd + i - 3] = buffer[i];
      }
      actaddrreceivecmd += 250;
    } else if (cmddetail == 1) {
      // first block
      uint16_t addr = buffer[3] + (buffer[4] << 8);
      actaddrreceivecmd = addr;
      PlatformManager::getInstance().log(LOG_INFO, TAG, "first block: %x",
                                         actaddrreceivecmd);
      for (uint8_t i = 5; i < 253; i++) {
        ram[actaddrreceivecmd + i - 5] = buffer[i];
      }
      actaddrreceivecmd += 253 - 5;
    } else if (cmddetail == 2) {
      // last block
      uint8_t len = buffer[3];
      PlatformManager::getInstance().log(LOG_INFO, TAG, "last block: %x",
                                         actaddrreceivecmd);
      for (uint8_t i = 4; i < (len + 4); i++) {
        ram[actaddrreceivecmd + i - 4] = buffer[i];
      }
      actaddrreceivecmd += len;
      setVarTab(actaddrreceivecmd);
    }
    cpu->cpuhalted = false;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "leave receivedata");
    setType4Notification();
    return 4;
  }
  case ExtCmd::RESTORE:
    if (buffer[1] == 1) {
      // restore + run/stop
      cpu->setMem(0xdc00, 0);
      cpu->setKeycodes(0x7f, 0);
    }
    cpu->restorenmi = true;
    return 0;
  case ExtCmd::SHOWREG:
    setType2Notification();
    PlatformManager::getInstance().log(LOG_INFO, TAG, "cpuRunning %s",
                                       type2notification.cpuRunning ? "true"
                                                                    : "false");
    PlatformManager::getInstance().log(
        LOG_INFO, TAG, "pc = %x, a = %x, x = %x, y = %x, sr = %x",
        type2notification.pc, type2notification.a, type2notification.x,
        type2notification.y, type2notification.sr);
    PlatformManager::getInstance().log(
        LOG_INFO, TAG, "d011 = %x, d016 = %x, d018 = %x",
        type2notification.d011, type2notification.d016, type2notification.d018);
    PlatformManager::getInstance().log(
        LOG_INFO, TAG, "d019 = %x, d01a = %x, register1 = %x",
        type2notification.d019, type2notification.d01a,
        type2notification.register1);
    PlatformManager::getInstance().log(
        LOG_INFO, TAG, "dc0d = %x, dc0e = %x, dc0f = %x",
        type2notification.dc0d, type2notification.dc0e, type2notification.dc0f);
    PlatformManager::getInstance().log(
        LOG_INFO, TAG, "dd0d = %x, dd0e = %x, dd0f = %x",
        type2notification.dd0d, type2notification.dd0e, type2notification.dd0f);
    return 2;
  case ExtCmd::SHOWMEM: {
    uint16_t addr = buffer[3] + (buffer[4] << 8);
    // use addr also as debugging start address
    cpu->debugstartaddr = addr;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "addr: %x", addr);
    setType3Notification(addr);
    for (uint8_t i = 0; i < NOTIFICATIONTYPE3NUMOFBYTES / 8; i++) {
      uint8_t j = i * 8;
      PlatformManager::getInstance().log(
          LOG_INFO, TAG, "mem[%d]: %d %d %d %d %d %d %d %d", j,
          type3notification.mem[j], type3notification.mem[j + 1],
          type3notification.mem[j + 2], type3notification.mem[j + 3],
          type3notification.mem[j + 4], type3notification.mem[j + 5],
          type3notification.mem[j + 6], type3notification.mem[j + 7]);
    }
    return 3;
  }
  case ExtCmd::RESET:
    cpu->cpuhalted = true;
    cpu->initMemAndRegs();
    cpu->vic.initVarsAndRegs();
    cpu->cia1.init(true);
    cpu->cia2.init(false);
    cpu->sid.init();
    cpu->cpuhalted = false;
    return 0;
  case ExtCmd::JOYSTICKMODE1:
    cpu->joystickmode = 1;
    cpu->kbjoystickmode = 0;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "joystickmode = %x",
                                       cpu->joystickmode);
    setType1Notification();
    return 1;
  case ExtCmd::JOYSTICKMODE2:
    cpu->joystickmode = 2;
    cpu->kbjoystickmode = 0;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "joystickmode = %x",
                                       cpu->joystickmode);
    setType1Notification();
    return 1;
  case ExtCmd::JOYSTICKMODEOFF:
    cpu->joystickmode = 0;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "joystickmode = %x",
                                       cpu->joystickmode);
    setType1Notification();
    return 1;
  case ExtCmd::KBJOYSTICKMODE1:
    cpu->kbjoystickmode = 1;
    cpu->joystickmode = 0;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "kbjoystickmode = %x",
                                       cpu->kbjoystickmode);
    return 0;
  case ExtCmd::KBJOYSTICKMODE2:
    cpu->kbjoystickmode = 2;
    cpu->joystickmode = 0;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "kbjoystickmode = %x",
                                       cpu->kbjoystickmode);
    return 0;
  case ExtCmd::KBJOYSTICKMODEOFF:
    cpu->kbjoystickmode = 0;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "kbjoystickmode = %x",
                                       cpu->kbjoystickmode);
    return 0;
  case ExtCmd::GETSTATUS:
    // just send type 1 notification
    PlatformManager::getInstance().log(LOG_INFO, TAG, "send status to client");
    setType1Notification();
    return 1;
  case ExtCmd::SWITCHDEACTIVATETEMP:
    cpu->deactivateTemp = !cpu->deactivateTemp;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "deactivateTemp = %x",
                                       cpu->deactivateTemp);
    setType1Notification();
    return 1;
  case ExtCmd::SENDRAWKEYS:
    sendrawkeycodes = !sendrawkeycodes;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "sendrawkeycodes = %x",
                                       sendrawkeycodes);
    setType1Notification();
    return 1;
  case ExtCmd::SWITCHDEBUG:
    cpu->debug = !cpu->debug;
    cpu->debuggingstarted = false;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "debug = %x", cpu->debug);
    setType1Notification();
    return 1;
  case ExtCmd::SWITCHPERF:
    cpu->perf.store(!cpu->perf.load(std::memory_order_acquire),
                    std::memory_order_release);
    PlatformManager::getInstance().log(
        LOG_INFO, TAG, "perf = %x", cpu->perf.load(std::memory_order_acquire));
    setType1Notification();
    return 1;
  case ExtCmd::SWITCHDETECTRELEASEKEY:
    cpu->detectreleasekey = !cpu->detectreleasekey;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "detectreleasekey = %x",
                                       cpu->detectreleasekey);
    setType1Notification();
    return 1;
  case ExtCmd::GETBATTERYVOLTAGE: {
    uint16_t voltage = cpu->batteryVoltage.load(std::memory_order_acquire);
    setType5Notification(voltage & 0xff, (voltage >> 8) & 0xff);
    return 5;
  }
  case ExtCmd::POWEROFF:
    cpu->poweroff.store(true, std::memory_order_release);
    return 0;
  case ExtCmd::SETVOLUME:
    cpu->sid.setEmuVolume(buffer[1]);
    return 0;
  case ExtCmd::INCVOLUME: {
    int16_t newVolume = cpu->sid.getEmuVolume() + buffer[1];
    if (newVolume > 255) {
      newVolume = 255;
    }
    cpu->sid.setEmuVolume(newVolume);
    dispVolume();
    setType6Notification((uint8_t)newVolume);
    return 6;
  }
  case ExtCmd::DECVOLUME: {
    int16_t newVolume = cpu->sid.getEmuVolume() - buffer[1];
    if (newVolume < 0) {
      newVolume = 0;
    }
    cpu->sid.setEmuVolume(newVolume);
    dispVolume();
    setType6Notification((uint8_t)newVolume);
    return 6;
  }
  case ExtCmd::WRITETEXT: {
    if (!isBasicInputMode()) {
      return 0;
    }
    PlatformManager::getInstance().log(LOG_INFO, TAG, "execute writetext");
    uint16_t addr = 0xc000;
    int16_t sizebuffer = strlen((const char *)&(buffer[1]));
    memcpy(&ram[addr], &(buffer[1]), sizebuffer);
    while (sizebuffer > 0) {
      uint16_t oldaddr = addr;
      uint8_t inc = sizebuffer > 250 ? 250 : sizebuffer;
      addr += inc;
      uint8_t ch = ram[addr];
      ram[addr] = '\0';
      cpu->exeSubroutine(0xab1e, (uint8_t)(oldaddr & 0xff), 0,
                         (uint8_t)((oldaddr >> 8) & 0xff));
      ram[addr] = ch;
      sizebuffer -= 250;
    }
    return 0;
  }
  }
  return 0;
}
