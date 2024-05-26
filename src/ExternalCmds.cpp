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
#include "loadactions.h"
#include <esp_log.h>

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
  SETVARTAB = 15,
  RESET = 20,
  GETSTATUS = 21,
  SWITCHFRAMECOLORREFRESH = 22,
  SWITCHCIA2 = 23,
  JOYEMULMODE = 24
};

void ExternalCmds::init(uint8_t *ram, CPUC64 *cpu) {
  this->ram = ram;
  this->cpu = cpu;
}

void ExternalCmds::setType1Notification() {
  type1notification.type = 1;
  type1notification.joymode = cpu->joystickmode;
  type1notification.refreshframecolor = cpu->refreshframecolor;
  type1notification.switchonoffcia2 = cpu->deactivatecia2;
  type1notification.joyemulmode = cpu->joystickemulmode;
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
}

void ExternalCmds::setVarTab(uint16_t addr) {
  // set VARTAB
  ram[0x2d] = addr % 256;
  ram[0x2e] = addr / 256;
  // clr
  cpu->setPC(0xa52a);
}

uint8_t ExternalCmds::executeExternalCmd(uint8_t *buffer) {
  uint16_t addr;
  bool fileloaded;
  ExtCmd cmd = static_cast<ExtCmd>(buffer[0]);
  switch (cmd) {
  case ExtCmd::NOEXTCMD:
    return 0;
  case ExtCmd::LOAD:
    ESP_LOGI(TAG, "load from sdcard...");
    cpu->cpuhalted = true;
    fileloaded = false;
    if (sdcard.init()) {
      uint8_t cury = ram[0xd6];
      uint8_t curx = ram[0xd3];
      addr = sdcard.load(SD_MMC, ram + 0x0400 + cury * 40 + curx, ram);
      if (addr == 0) {
        ESP_LOGI(TAG, "error loading file");
      } else {
        setVarTab(addr);
        fileloaded = true;
      }
    } else {
      ESP_LOGI(TAG, "error init sdcard");
    }
    addr = src_loadactions_prg[0] + (src_loadactions_prg[1] << 8);
    memcpy(ram + addr, src_loadactions_prg + 2, src_loadactions_prg_len - 2);
    if (fileloaded) {
      fileloaded = false;
      cpu->exeSubroutine(addr, 1, 0, 0);
    } else {
      cpu->exeSubroutine(addr, 0, 0, 0);
    }
    cpu->cpuhalted = false;
    return 0;
  case ExtCmd::RECEIVEDATA:
    ESP_LOGI(TAG, "enter receivedata");
    cpu->cpuhalted = true;
    // simple "protocol":
    // - byte 4: number of bytes which were received (max 249 bytes)
    // - byte 5-6: address the bytes must be written to
    // - byte 7-: data bytes
    addr = buffer[4] + (buffer[5] << 8);
    ESP_LOGI(TAG, "address data will be transfered to: %x", addr);
    for (uint8_t i = 0; i < buffer[3]; i++) {
      ram[addr++] = buffer[i + 6];
    }
    cpu->cpuhalted = false;
    ESP_LOGI(TAG, "leave receivedata");
    return 0;
  case ExtCmd::SETVARTAB:
    cpu->cpuhalted = true;
    addr = buffer[3] + (buffer[4] << 8);
    setVarTab(addr);
    cpu->cpuhalted = false;
    return 0;
  case ExtCmd::SHOWREG:
    setType2Notification();
    ESP_LOGI(TAG, "cpuRunning %s",
             type2notification.cpuRunning ? "false" : "true");
    ESP_LOGI(TAG, "pc = %x, a = %x, x = %x, y = %x, sr = %x",
             type2notification.pc, type2notification.a, type2notification.x,
             type2notification.y, type2notification.sr);
    ESP_LOGI(TAG, "vic[0xd011] = %x, vic[0xd016] = %x, vic[0xd018] = %x",
             type2notification.d011, type2notification.d016,
             type2notification.d018);
    ESP_LOGI(TAG, "vic[0xd019] = %x, vic[0xd01a] = %x", type2notification.d019,
             type2notification.d01a);
    return 2;
  case ExtCmd::SHOWMEM: // todo
    return 0;
  case ExtCmd::RESET:
    cpu->cpuhalted = true;
    cpu->initMemAndRegs();
    cpu->vic->initVarsAndRegs();
    cpu->cia1.init();
    cpu->cia2.init();
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
    cpu->refreshframecolor = !cpu->refreshframecolor;
    ESP_LOGI(TAG, "refreshframecolor = %x", cpu->refreshframecolor);
    setType1Notification();
    return 1;
  case ExtCmd::SWITCHCIA2:
    cpu->deactivatecia2 = !cpu->deactivatecia2;
    ESP_LOGI(TAG, "deactivatecia2 = %x", cpu->deactivatecia2);
    setType1Notification();
    return 1;
  case ExtCmd::JOYEMULMODE:
    cpu->joystickemulmode = buffer[3];
    ESP_LOGI(TAG, "joystickemulmode = %x", cpu->joystickemulmode);
    setType1Notification();
    return 1;
  }
  return 0;
}
