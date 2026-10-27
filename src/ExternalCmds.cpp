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
#include "ExternalCmds.h"

#include "C64Sys.h"
#include "ExtCmd.h"
#include "ExtCmdQueue.h"
#include "Version.h"
#include "platform/PlatformManager.h"
#include <algorithm>
#include <cstring>
#include <string>

static const char *TAG = "ExternalCmds";

void ExternalCmds::init(uint8_t *ram, C64Sys *cpu) {
  this->ram = ram;
  this->cpu = cpu;
  sendrawkeycodes = false;
  liststartflag = true;
}

void ExternalCmds::setType1Notification() {
  type1notification.type = 1;
  type1notification.joymode = cpu->joystickmode;
  type1notification.deactivateTemp = cpu->deactivateTemp;
  type1notification.sendrawkeycodes = sendrawkeycodes;
  type1notification.switchdebug = cpu->debug;
  type1notification.switchperf = cpu->perf.load(std::memory_order_acquire);
  type1notification.switchdetectreleasekey = cpu->detectreleasekey;
  type1notification.volume = cpu->sid.getEmuVolume();
  type1notification.switchattached = cpu->floppy.d64attached;
}

void ExternalCmds::setType4Notification() { type4notification.type = 4; }

void ExternalCmds::setType5Notification(uint8_t batteryVolLow,
                                        uint8_t batteryVolHi) {
  type5notification.type = 5;
  type5notification.batteryVolLow = batteryVolLow;
  type5notification.batteryVolHi = batteryVolHi;
}

void ExternalCmds::setType6Notification() {
  type6notification.type = 6;
  type6notification.version = THMIC64KB_VERSION;
}

void ExternalCmds::setVarTab(uint16_t addr) {
  // set VARTAB
  ram[0x2d] = addr % 256;
  ram[0x2e] = addr / 256;
  // clr
  cpu->setPC(0xa52a);
}

std::string getFilename(uint8_t *ram, const std::string ext) {
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
  std::string filename;
  uint8_t p;
  while (((p = *cursorpos++) != 32) && (p != 160) && (filename.size() < 17)) {
    if ((p >= 1) && (p <= 26)) {
      filename.push_back(static_cast<char>(p + 96));
    } else if ((p >= 33) && (p <= 63)) {
      filename.push_back(static_cast<char>(p));
    }
  }
  filename += ext;
  return filename;
}

// the following method is not reliable
bool ExternalCmds::isBasicInputMode() {
  if ((cpu->getMem(1) & 7) != 7) {
    return false;
  }
  uint16_t irqVec = ram[0x0314] | (ram[0x0315] << 8);
  if (irqVec != 0xea31) {
    return false;
  }
  if ((cpu->getMem(0xd011) & 0x7f) != 27) {
    return false;
  }
  if ((cpu->getMem(0xd016) & 0x1f) != 8) {
    return false;
  }
  if (cpu->getMem(0xd018) != 20) {
    return false;
  }
  return true;
}

void ExternalCmds::dispVolume() {
  uint8_t volume = cpu->sid.getEmuVolume() * 100 / 256;
  uint8_t vol[] = {(uint8_t)(volume / 10 + '0'), (uint8_t)(volume % 10 + '0')};
  cpu->vic.drawDOIBox(vol, 37, 23, 2, 1, 1, 0, 4, 0);
}

void ExternalCmds::writeTextToC64Screen(uint16_t addr, int16_t sizebuffer) {
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
}

void ExternalCmds::setJ1() {
  cpu->joystickmode = 1;
  cpu->vic.drawDOIBox((uint8_t *)"\xa\x31", 37, 23, 2, 1, 1, 0, 3, 0);
  PlatformManager::getInstance().log(LOG_INFO, TAG, "joystickmode = %x",
                                     cpu->joystickmode);
}

void ExternalCmds::setJ2() {
  cpu->joystickmode = 2;
  cpu->vic.drawDOIBox((uint8_t *)"\xa\x32", 37, 23, 2, 1, 1, 0, 3, 0);
  PlatformManager::getInstance().log(LOG_INFO, TAG, "joystickmode = %x",
                                     cpu->joystickmode);
}

void ExternalCmds::setNoJoy() {
  cpu->joystickmode = 0;
  cpu->vic.drawDOIBox((uint8_t *)"\xe\xf", 37, 23, 2, 1, 1, 0, 3, 0);
  PlatformManager::getInstance().log(LOG_INFO, TAG, "joystickmode = %x",
                                     cpu->joystickmode);
}

void ExternalCmds::setJB1() {
  cpu->kbjoystickmode = 1;
  cpu->vic.drawDOIBox((uint8_t *)"\xa\x2\x31", 36, 22, 3, 1, 1, 0, 3, 1);
  PlatformManager::getInstance().log(LOG_INFO, TAG, "kbjoystickmode = %x",
                                     cpu->kbjoystickmode);
}

void ExternalCmds::setJB2() {
  cpu->kbjoystickmode = 2;
  cpu->vic.drawDOIBox((uint8_t *)"\xa\x2\x32", 36, 22, 3, 1, 1, 0, 3, 1);
  PlatformManager::getInstance().log(LOG_INFO, TAG, "kbjoystickmode = %x",
                                     cpu->kbjoystickmode);
}

void ExternalCmds::setNoJoyB() {
  cpu->kbjoystickmode = 0;
  cpu->vic.drawDOIBox((uint8_t *)"\xe\xf", 37, 22, 2, 1, 1, 0, 3, 1);
  PlatformManager::getInstance().log(LOG_INFO, TAG, "kbjoystickmode = %x",
                                     cpu->kbjoystickmode);
}

uint8_t ExternalCmds::executeNextExternalCmd() {
  if (ExtCmdQueue::getInstance().empty()) {
    return 0;
  }
  ExtCmdQueue::ExternalCmd *cmd = ExtCmdQueue::getInstance().waitTestAndDec();
  if (cmd->cmd == ExtCmd::WAIT) {
    if (cmd->param[0] == 0) {
      ExtCmdQueue::getInstance().pop();
    }
    return 0;
  }
  cmd = ExtCmdQueue::getInstance().pop();
  switch (cmd->cmd) {
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
    if (cpu->floppy.fsinitialized) {
      std::string filename = getFilename(ram, ".prg");
      addr = cpu->floppy.load(filename, ram);
      if (addr == 0) {
        PlatformManager::getInstance().log(LOG_INFO, TAG, "file not found");
      } else {
        setVarTab(addr);
        fileloaded = true;
      }
    } else {
      error = true;
      PlatformManager::getInstance().log(LOG_INFO, TAG,
                                         "file system not initialized");
    }
    addr = 0x342;
    if (fileloaded) {
      memcpy(&ram[addr], "\rLOADED\r\0", 9);
      writeTextToC64Screen(addr, 9);
    } else if (error) {
      memcpy(&ram[addr], "\rERROR\r\0", 8);
      writeTextToC64Screen(addr, 8);
    } else {
      memcpy(&ram[addr], "\rFILE NOT FOUND\r\0", 17);
      writeTextToC64Screen(addr, 17);
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
    if (cpu->floppy.fsinitialized) {
      std::string filename = getFilename(ram, ".prg");
      uint16_t startaddr = ram[43] + ram[44] * 256;
      uint16_t endaddr = ram[45] + ram[46] * 256;
      filesaved = cpu->floppy.save(filename, ram, startaddr, endaddr);
      if (!filesaved) {
        PlatformManager::getInstance().log(LOG_INFO, TAG, "error saving file");
      }
    } else {
      PlatformManager::getInstance().log(LOG_INFO, TAG,
                                         "file system not initialized");
    }
    uint16_t addr = 0x342;
    if (filesaved) {
      memcpy(&ram[addr], "\rSAVED\r\0", 8);
      writeTextToC64Screen(addr, 8);
    } else {
      memcpy(&ram[addr], "\rERROR\r\0", 8);
      writeTextToC64Screen(addr, 8);
    }
    cpu->cpuhalted = false;
    return 0;
  }
  case ExtCmd::LIST: {
    if (!isBasicInputMode()) {
      return 0;
    }
    cpu->cpuhalted = true;
    if (cpu->floppy.fsinitialized) {
      if (liststartflag) {
        const uint8_t start[] = "*** START ***\r\0";
        uint16_t addr = 0x342;
        memcpy(&ram[addr], start, 15);
        writeTextToC64Screen(addr, 15);
      } else {
        const uint8_t next[] = "*** NEXT ***\r\0";
        uint16_t addr = 0x342;
        memcpy(&ram[addr], next, 14);
        writeTextToC64Screen(addr, 14);
      }
      uint8_t cnt = 0;
      while (cnt < 23) {
        std::string filename;
        bool success = cpu->floppy.listnextentry(filename, liststartflag);
        if (!success) {
          PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                             "error reading entry");
          break;
        } else if (filename.empty()) {
          liststartflag = true;
          const uint8_t next[] = "*** END ***\r\0";
          uint16_t addr = 0x342;
          memcpy(&ram[addr], next, 13);
          writeTextToC64Screen(addr, 13);
          break;
        } else {
          liststartflag = false;
          cpu->floppy.rmPrgFromFilename(filename);
          std::transform(filename.begin(), filename.end(), filename.begin(),
                         ::toupper);
          uint16_t addr = 0x342;
          size_t len = std::min(filename.length(), static_cast<size_t>(16));
          std::memcpy(&ram[addr], filename.c_str(), len);
          ram[addr + len] = '\r';
          ram[addr + len + 1] = '\0';
          writeTextToC64Screen(addr, len + 2);
        }
        cnt++;
      }
    } else {
      PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                         "file system not initialized");
    }
    cpu->cpuhalted = false;
    return 0;
  }
  case ExtCmd::ATTACHD64: {
    if (cpu->floppy.fsinitialized) {
      std::string dirname(reinterpret_cast<char *>(&cmd->param[2]));
      dirname += ".d64";
      PlatformManager::getInstance().log(
          LOG_INFO, TAG, "try to attach d64 file %s", dirname.c_str());
      bool d64attached = cpu->floppy.attach(dirname);
      if (!d64attached) {
        cpu->vic.drawDOIBox((uint8_t *)"\xe\xf", 37, 23, 2, 1, 1, 0, 4, 0);
        PlatformManager::getInstance().log(LOG_INFO, TAG, "d64 file not found");
      } else {
        cpu->vic.drawDOIBox((uint8_t *)"\xf\xb", 37, 23, 2, 1, 1, 0, 4, 0);
      }
    }
    return 0;
  }
  case ExtCmd::DETACHD64: {
    PlatformManager::getInstance().log(LOG_INFO, TAG, "detach d64 file");
    if (cpu->floppy.fsinitialized) {
      cpu->floppy.detach();
    }
    cpu->vic.drawDOIBox((uint8_t *)"\xe\xf", 37, 23, 2, 1, 1, 0, 4, 0);
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
    uint8_t cmddetail = cmd->param[0];
    if (cmddetail == 0) {
      // next block
      PlatformManager::getInstance().log(LOG_INFO, TAG, "next block: %x",
                                         actaddrreceivecmd);
      for (uint8_t i = 2; i < 252; i++) {
        ram[actaddrreceivecmd + i - 2] = cmd->param[i];
      }
      actaddrreceivecmd += 250;
    } else if (cmddetail == 1) {
      // first block
      uint16_t addr = cmd->param[2] + (cmd->param[3] << 8);
      actaddrreceivecmd = addr;
      PlatformManager::getInstance().log(LOG_INFO, TAG, "first block: %x",
                                         actaddrreceivecmd);
      for (uint8_t i = 4; i < 252; i++) {
        ram[actaddrreceivecmd + i - 4] = cmd->param[i];
      }
      actaddrreceivecmd += 253 - 5;
    } else if (cmddetail == 2) {
      // last block
      uint8_t len = cmd->param[2];
      PlatformManager::getInstance().log(LOG_INFO, TAG, "last block: %x",
                                         actaddrreceivecmd);
      for (uint8_t i = 3; i < (len + 3); i++) {
        ram[actaddrreceivecmd + i - 3] = cmd->param[i];
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
    if (cmd->param[0] == 1) {
      // restore + run/stop
      cpu->setMem(0xdc00, 0);
      cpu->actInGameKeycode = {0, 0x7f, 0};
      cpu->actInGameKeycodeChosen.store(true, std::memory_order_release);
      cpu->actInGameKeycodeCnt.store(3, std::memory_order_release);
    }
    cpu->restorenmi = true;
    return 0;
  case ExtCmd::RESET:
    cpu->cpuhalted = true;
    cpu->initMemAndRegs();
    cpu->vic.initVarsAndRegs();
    cpu->cia1.init(true);
    cpu->cia2.init(false);
    cpu->sid.initSID();
    cpu->floppy.init(8);
    cpu->cpuhalted = false;
    return 0;
  case ExtCmd::JOYSTICKMODE1:
    setJ1();
    if (cpu->kbjoystickmode != 0) {
      setJB2();
    }
    return 0;
  case ExtCmd::JOYSTICKMODE2:
    setJ2();
    if (cpu->kbjoystickmode != 0) {
      setJB1();
    }
    return 0;
  case ExtCmd::JOYSTICKMODEOFF:
    setNoJoy();
    return 0;
  case ExtCmd::KBJOYSTICKMODE1:
    setJB1();
    if (cpu->joystickmode != 0) {
      setJ2();
    }
    return 0;
  case ExtCmd::KBJOYSTICKMODE2:
    setJB2();
    if (cpu->joystickmode != 0) {
      setJ1();
    }
    return 0;
  case ExtCmd::KBJOYSTICKMODEOFF:
    setNoJoyB();
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
    return 0;
  case ExtCmd::SENDRAWKEYS:
    sendrawkeycodes = !sendrawkeycodes;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "sendrawkeycodes = %x",
                                       sendrawkeycodes);
    return 0;
  case ExtCmd::SWITCHDEBUG:
    cpu->debug = !cpu->debug;
    if (cpu->debug) {
      cpu->debugNumOfSteps = 1000;
    } else {
      cpu->debugNumOfSteps = 0;
    }
    PlatformManager::getInstance().log(LOG_INFO, TAG, "debug = %x", cpu->debug);
    return 0;
  case ExtCmd::SWITCHPERF:
    cpu->perf.store(!cpu->perf.load(std::memory_order_acquire),
                    std::memory_order_release);
    PlatformManager::getInstance().log(
        LOG_INFO, TAG, "perf = %x", cpu->perf.load(std::memory_order_acquire));
    return 0;
  case ExtCmd::SWITCHDETECTRELEASEKEY:
    cpu->detectreleasekey = !cpu->detectreleasekey;
    cpu->keyboard->setDetectReleasekey(cpu->detectreleasekey);
    PlatformManager::getInstance().log(LOG_INFO, TAG, "detectreleasekey = %x",
                                       cpu->detectreleasekey);
    return 0;
  case ExtCmd::GETBATTERYVOLTAGE: {
    uint16_t voltage = cpu->batteryVoltage.load(std::memory_order_acquire);
    setType5Notification(voltage & 0xff, (voltage >> 8) & 0xff);
    return 5;
  }
  case ExtCmd::GETVERSION: {
    setType6Notification();
    PlatformManager::getInstance().log(LOG_INFO, TAG, "THMIC64KB version: %d",
                                       THMIC64KB_VERSION);
    return 6;
  }
  case ExtCmd::POWEROFF:
    cpu->poweroff.store(true, std::memory_order_release);
    return 0;
  case ExtCmd::SETVOLUME:
    cpu->sid.setEmuVolume(cmd->param[0]);
    PlatformManager::getInstance().log(LOG_INFO, TAG, "volume = %d",
                                       cpu->sid.getEmuVolume());
    dispVolume();
    return 0;
  case ExtCmd::INCVOLUME: {
    int16_t newVolume = cpu->sid.getEmuVolume() + cmd->param[0];
    if (newVolume > 255) {
      newVolume = 255;
    }
    cpu->sid.setEmuVolume(newVolume);
    PlatformManager::getInstance().log(LOG_INFO, TAG, "volume = %d",
                                       cpu->sid.getEmuVolume());
    dispVolume();
    return 0;
  }
  case ExtCmd::DECVOLUME: {
    int16_t newVolume = cpu->sid.getEmuVolume() - cmd->param[0];
    if (newVolume < 0) {
      newVolume = 0;
    }
    cpu->sid.setEmuVolume(newVolume);
    PlatformManager::getInstance().log(LOG_INFO, TAG, "volume = %d",
                                       cpu->sid.getEmuVolume());
    dispVolume();
    return 0;
  }
  case ExtCmd::WRITETEXT: {
    if (!isBasicInputMode()) {
      return 0;
    }
    PlatformManager::getInstance().log(LOG_INFO, TAG, "execute writetext");
    uint16_t addr = 0xc000;
    int16_t sizebuffer = strlen((const char *)&(cmd->param[2]));
    memcpy(&ram[addr], &(cmd->param[2]), sizebuffer);
    writeTextToC64Screen(addr, sizebuffer);
    return 0;
  }
  case ExtCmd::WRITEOSD: {
    PlatformManager::getInstance().log(LOG_INFO, TAG, "execute writeosd");
    uint16_t duration = cmd->param[8] + (cmd->param[9] << 8);
    cpu->vic.drawDOIBox(&(cmd->param[11]), cmd->param[2], cmd->param[3],
                        cmd->param[4], cmd->param[5], cmd->param[6],
                        cmd->param[7], duration, cmd->param[10]);
    return 0;
  }
  case ExtCmd::PAUSE: {
    if (cpu->cpuhalted) {
      cpu->cpuhalted = false;
    } else {
      cpu->cpuhalted = true;
    }
    PlatformManager::getInstance().log(LOG_INFO, TAG, "execute pause");
    return 0;
  }
  case ExtCmd::AUTOSTART: {
    PlatformManager::getInstance().log(LOG_INFO, TAG, "execute autostart");
    if (cpu->floppy.fsinitialized) {
      cpu->cpuhalted = true;
      std::string filename(reinterpret_cast<char *>(&cmd->param[2]));
      filename += ".prg";
      uint16_t addr = cpu->floppy.load(filename, ram);
      cpu->vic.display->reconfigureSPI();
      if (addr != 0) {
        cpu->joystickOnlyModeState = JoystickOnlyModeState::RUN;
        setVarTab(addr);
        ram[0xd3] = 0;
        ram[0x0277] = 'R';
        ram[0x0278] = 'U';
        ram[0x0279] = 'N';
        ram[0x027A] = ':';
        ram[0x027B] = 0x0d;
        ram[0x00C6] = 5;
      }
      cpu->cpuhalted = false;
    }
    return 0;
  }
  case ExtCmd::SPECIAL1: {
    cpu->vic.display->setSpecial1();
    PlatformManager::getInstance().log(LOG_INFO, TAG, "execute special1");
    return 0;
  }
  case ExtCmd::SPECIAL2: {
    cpu->vic.display->setSpecial2();
    PlatformManager::getInstance().log(LOG_INFO, TAG, "execute special2");
    return 0;
  }
  case ExtCmd::SENDKEY: {
    PlatformManager::getInstance().log(LOG_INFO, TAG, "execute sendkey");
    cpu->actInGameKeycode = {cmd->param[0], cmd->param[1], cmd->param[2]};
    cpu->actInGameKeycodeChosen.store(true, std::memory_order_release);
    cpu->actInGameKeycodeCnt.store(cmd->param[3] > 0 ? cmd->param[3] : 3,
                                   std::memory_order_release);
    return 0;
  }
  case ExtCmd::SENDJOY: {
    // PlatformManager::getInstance().log(LOG_INFO, TAG, "execute sendjoy: %x",
    //                                    cmd->param[0]);
    cpu->restJoyValue.store(cmd->param[0], std::memory_order_release);
    return 0;
  }
  case ExtCmd::SAVESNAPSHOT: {
    PlatformManager::getInstance().log(LOG_INFO, TAG, "execute savesnapshot");
    cpu->snapshot.save(cpu);
    return 0;
  }
  case ExtCmd::LOADSNAPSHOT: {
    PlatformManager::getInstance().log(LOG_INFO, TAG, "execute loadsnapshot");
    cpu->snapshot.load(cpu);
    return 0;
  }
  case ExtCmd::WAIT: {
    // WAIT is handled before case statement
    return 0;
  }
  }
  return 0;
}
