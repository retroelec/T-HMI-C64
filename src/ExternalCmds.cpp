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
#include "CPUC64.h"
#include "jllog.h"
#include "listactions.h"
#include "loadactions.h"
#include "saveactions.h"

static const char *TAG = "ExternalCmds";

enum class ExtCmd {
  NOEXTCMD = 0,
  JOYSTICKMODE1 = 1,
  JOYSTICKMODE2 = 2,
  KBJOYSTICKMODE1 = 3,
  KBJOYSTICKMODE2 = 4,
  JOYSTICKMODEOFF = 5,
  KBJOYSTICKMODEOFF = 6,
  LOAD = 11,
  RECEIVEDATA = 12,
  SHOWREG = 13,
  SHOWMEM = 14,
  RESTORE = 15,
  RESET = 20,
  GETSTATUS = 21,
  SWITCHFRAMECOLORREFRESH = 22,
  SENDRAWKEYS = 24,
  SWITCHDEBUG = 25,
  SWITCHPERF = 26,
  SWITCHDETECTRELEASEKEY = 27,
  GETBATTERYVOLTAGE = 29,
  POWEROFF = 30,
  SAVE = 31,
  LIST = 32
};

void ExternalCmds::init(uint8_t *ram, CPUC64 *cpu) {
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
  type1notification.switchperf = cpu->perf;
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
  for (uint8_t i = 0; i < BLENOTIFICATIONTYPE3NUMOFBYTES; i++) {
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

void ExternalCmds::setVarTab(uint16_t addr) {
  // set VARTAB
  ram[0x2d] = addr % 256;
  ram[0x2e] = addr / 256;
  // clr
  cpu->setPC(0xa52a);
}

uint8_t ExternalCmds::executeExternalCmd(uint8_t *buffer) {
  ExtCmd cmd = static_cast<ExtCmd>(buffer[0]);
  switch (cmd) {
  case ExtCmd::NOEXTCMD:
    return 0;
  case ExtCmd::LOAD: {
    ESP_LOGI(TAG, "load from sdcard...");
    cpu->cpuhalted = true;
    bool fileloaded = false;
    bool error = false;
    uint16_t addr;
    if (sdcard.init()) {
      addr = sdcard.load(SD_MMC, ram);
      if (addr == 0) {
        ESP_LOGI(TAG, "file not found");
      } else {
        setVarTab(addr);
        fileloaded = true;
      }
    } else {
      error = true;
      ESP_LOGI(TAG, "error init sdcard");
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
    ESP_LOGI(TAG, "save to sdcard...");
    cpu->cpuhalted = true;
    bool filesaved = false;
    if (sdcard.init()) {
      filesaved = sdcard.save(SD_MMC, ram);
      if (!filesaved) {
        ESP_LOGI(TAG, "error saving file");
      }
    } else {
      ESP_LOGI(TAG, "error init sdcard");
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
    ESP_LOGI(TAG, "list sdcard...");
    cpu->cpuhalted = true;
    if (sdcard.init()) {
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
        bool success = sdcard.listnextentry(SD_MMC, filename, liststartflag);
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
            ESP_LOGE(TAG, "error reading entry");
          }
          liststartflag = true;
          break;
        }
        cnt++;
      }
    } else {
      ESP_LOGE(TAG, "error init sdcard");
    }
    cpu->cpuhalted = false;
    return 0;
  }
  case ExtCmd::RECEIVEDATA: {
    ESP_LOGI(TAG, "enter receivedata");
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
      ESP_LOGI(TAG, "next block: %x", actaddrreceivecmd);
      for (uint8_t i = 3; i < 253; i++) {
        ram[actaddrreceivecmd + i - 3] = buffer[i];
      }
      actaddrreceivecmd += 250;
    } else if (cmddetail == 1) {
      // first block
      uint16_t addr = buffer[3] + (buffer[4] << 8);
      actaddrreceivecmd = addr;
      ESP_LOGI(TAG, "first block: %x", actaddrreceivecmd);
      for (uint8_t i = 5; i < 253; i++) {
        ram[actaddrreceivecmd + i - 5] = buffer[i];
      }
      actaddrreceivecmd += 253 - 5;
    } else if (cmddetail == 2) {
      // last block
      uint8_t len = buffer[3];
      ESP_LOGI(TAG, "last block: %x", actaddrreceivecmd);
      for (uint8_t i = 4; i < (len + 4); i++) {
        ram[actaddrreceivecmd + i - 4] = buffer[i];
      }
      actaddrreceivecmd += len;
      setVarTab(actaddrreceivecmd);
    }
    cpu->cpuhalted = false;
    ESP_LOGI(TAG, "leave receivedata");
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
    ESP_LOGI(TAG, "cpuRunning %s",
             type2notification.cpuRunning ? "true" : "false");
    ESP_LOGI(TAG, "pc = %x, a = %x, x = %x, y = %x, sr = %x",
             type2notification.pc, type2notification.a, type2notification.x,
             type2notification.y, type2notification.sr);
    ESP_LOGI(TAG, "d011 = %x, d016 = %x, d018 = %x", type2notification.d011,
             type2notification.d016, type2notification.d018);
    ESP_LOGI(TAG, "d019 = %x, d01a = %x, register1 = %x",
             type2notification.d019, type2notification.d01a,
             type2notification.register1);
    ESP_LOGI(TAG, "dc0d = %x, dc0e = %x, dc0f = %x", type2notification.dc0d,
             type2notification.dc0e, type2notification.dc0f);
    ESP_LOGI(TAG, "dd0d = %x, dd0e = %x, dd0f = %x", type2notification.dd0d,
             type2notification.dd0e, type2notification.dd0f);
    return 2;
  case ExtCmd::SHOWMEM: {
    uint16_t addr = buffer[3] + (buffer[4] << 8);
    // use addr also as debugging start address
    cpu->debugstartaddr = addr;
    ESP_LOGI(TAG, "addr: %x", addr);
    setType3Notification(addr);
    for (uint8_t i = 0; i < BLENOTIFICATIONTYPE3NUMOFBYTES / 8; i++) {
      uint8_t j = i * 8;
      ESP_LOGI(TAG, "mem[%d]: %d %d %d %d %d %d %d %d", j,
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
    cpu->vic->initVarsAndRegs();
    cpu->cia1.init(true);
    cpu->cia2.init(false);
    cpu->cpuhalted = false;
    return 0;
  case ExtCmd::JOYSTICKMODE1:
    cpu->joystickmode = 1;
    cpu->kbjoystickmode = 0;
    ESP_LOGI(TAG, "joystickmode = %x", cpu->joystickmode);
    setType1Notification();
    return 1;
  case ExtCmd::JOYSTICKMODE2:
    cpu->joystickmode = 2;
    cpu->kbjoystickmode = 0;
    ESP_LOGI(TAG, "joystickmode = %x", cpu->joystickmode);
    setType1Notification();
    return 1;
  case ExtCmd::JOYSTICKMODEOFF:
    cpu->joystickmode = 0;
    ESP_LOGI(TAG, "joystickmode = %x", cpu->joystickmode);
    setType1Notification();
    return 1;
  case ExtCmd::KBJOYSTICKMODE1:
    cpu->kbjoystickmode = 1;
    cpu->joystickmode = 0;
    ESP_LOGI(TAG, "kbjoystickmode = %x", cpu->kbjoystickmode);
    return 0;
  case ExtCmd::KBJOYSTICKMODE2:
    cpu->kbjoystickmode = 2;
    cpu->joystickmode = 0;
    ESP_LOGI(TAG, "kbjoystickmode = %x", cpu->kbjoystickmode);
    return 0;
  case ExtCmd::KBJOYSTICKMODEOFF:
    cpu->kbjoystickmode = 0;
    ESP_LOGI(TAG, "kbjoystickmode = %x", cpu->kbjoystickmode);
    return 0;
  case ExtCmd::GETSTATUS:
    // just send type 1 notification
    ESP_LOGI(TAG, "send status to BLE client");
    setType1Notification();
    return 1;
  case ExtCmd::SWITCHFRAMECOLORREFRESH:
    cpu->deactivateTemp = !cpu->deactivateTemp;
    ESP_LOGI(TAG, "deactivateTemp = %x", cpu->deactivateTemp);
    setType1Notification();
    return 1;
  case ExtCmd::SENDRAWKEYS:
    sendrawkeycodes = !sendrawkeycodes;
    ESP_LOGI(TAG, "sendrawkeycodes = %x", sendrawkeycodes);
    setType1Notification();
    return 1;
  case ExtCmd::SWITCHDEBUG:
    cpu->debug = !cpu->debug;
    cpu->debuggingstarted = false;
    ESP_LOGI(TAG, "debug = %x", cpu->debug);
    setType1Notification();
    return 1;
  case ExtCmd::SWITCHPERF:
    cpu->perf = !cpu->perf;
    ESP_LOGI(TAG, "perf = %x", cpu->perf);
    setType1Notification();
    return 1;
  case ExtCmd::SWITCHDETECTRELEASEKEY:
    cpu->detectreleasekey = !cpu->detectreleasekey;
    ESP_LOGI(TAG, "detectreleasekey = %x", cpu->detectreleasekey);
    setType1Notification();
    return 1;
  case ExtCmd::GETBATTERYVOLTAGE: {
    uint32_t voltage = cpu->batteryVoltage;
    setType5Notification(voltage & 0xff, (voltage >> 8) & 0xff);
    return 5;
  }
#ifdef BOARD_T_HMI
  case ExtCmd::POWEROFF:
    cpu->poweroff = true;
    return 0;
#endif
  }
  return 0;
}
