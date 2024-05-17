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
#include "CBEServiceLocator.h"
#include "loadactions.h"
#include <esp_log.h>

static const char *TAG = "ExternalCmds";

void ExternalCmds::init(uint8_t *ram) { this->ram = ram; }

void ExternalCmds::setType1Notification() {
  type1notification.type = 1;
  type1notification.joymode = CBEServiceLocator::getCPUC64()->joystickmode;
  type1notification.kbjoymode = CBEServiceLocator::getCPUC64()->kbjoystickmode;
  type1notification.refreshframecolor =
      CBEServiceLocator::getCPUC64()->refreshframecolor;
  type1notification.switchonoffcia2 = 0; // todo
  type1notification.joyemulmode = 0;     // todo
  type1notification.cpuRunning = !CBEServiceLocator::getCPUC64()->cpuhalted;
}

uint8_t ExternalCmds::executeExternalCmd(ExtCmd cmd) {
  uint8_t low;
  uint8_t hi;
  uint8_t numofbytes;
  uint16_t addr;
  bool fileloaded;
  switch (cmd) {
  case ExtCmd::NOEXTCMD:
    return 0;
  case ExtCmd::LOAD:
    ESP_LOGI(TAG, "load from sdcard...");
    CBEServiceLocator::getCPUC64()->cpuhalted = true;
    fileloaded = false;
    if (sdcard.init()) {
      uint8_t cury = ram[0xd6];
      uint8_t curx = ram[0xd3];
      addr = sdcard.load(SD_MMC, ram + 0x0400 + cury * 40 + curx, ram);
      if (addr == 0) {
        ESP_LOGI(TAG, "error loading file");
      } else {
        // set VARTAB
        ram[0x2d] = addr % 256;
        ram[0x2e] = addr / 256;
        // clr
        CBEServiceLocator::getCPUC64()->setPC(0xa52a);
        fileloaded = true;
      }
    } else {
      ESP_LOGI(TAG, "error init sdcard");
    }
    addr = src_loadactions_prg[0] + (src_loadactions_prg[1] << 8);
    memcpy(ram + addr, src_loadactions_prg + 2, src_loadactions_prg_len - 2);
    if (fileloaded) {
      fileloaded = false;
      CBEServiceLocator::getCPUC64()->exeSubroutine(addr, 1, 0, 0);
    } else {
      CBEServiceLocator::getCPUC64()->exeSubroutine(addr, 0, 0, 0);
    }
    CBEServiceLocator::getCPUC64()->cpuhalted = false;
    return 33;
  case ExtCmd::RECEIVEDATA:
    ESP_LOGI(TAG, "enter hostcmd_receivedata");
    CBEServiceLocator::getCPUC64()->cpuhalted = true;
    while (true) {
      // simple "protocol":
      // - first byte = number of bytes which will be sent (max 248 bytes)
      // - next two bytes = address the bytes must be written to
      // - afterwards send announced number of bytes
      // - if numofbytes == 255 then transfer is finished -> init basic
      while (!CBEServiceLocator::getBLEKB()->getData(&numofbytes)) {
      }
      ESP_LOGI(TAG, "number of bytes to transfer: %x", numofbytes);
      if (numofbytes == 255) {
        break;
      }
      while (!CBEServiceLocator::getBLEKB()->getData(&low)) {
      }
      while (!CBEServiceLocator::getBLEKB()->getData(&hi)) {
      }
      addr = low + (hi << 8);
      ESP_LOGI(TAG, "address data will be transfered to: %x", low + (hi << 8));
      for (uint8_t i = 0; i < numofbytes; i++) {
        while (!CBEServiceLocator::getBLEKB()->getData(&ram[addr])) {
        }
        addr++;
      }
      vTaskDelay(10);
    }
    // set VARTAB
    ram[0x2d] = addr % 256;
    ram[0x2e] = addr / 256;
    // clr
    CBEServiceLocator::getCPUC64()->setPC(0xa52a);
    CBEServiceLocator::getCPUC64()->cpuhalted = false;
    ESP_LOGI(TAG, "leave hostcmd_receivedata");
    return 0;
  case ExtCmd::SHOWREG:
    ESP_LOGI(TAG, "pc = %x, a = %x, x = %x, y = %x, sr = %B",
             CBEServiceLocator::getCPUC64()->getPC(),
             CBEServiceLocator::getCPUC64()->getA(),
             CBEServiceLocator::getCPUC64()->getX(),
             CBEServiceLocator::getCPUC64()->getY(),
             CBEServiceLocator::getCPUC64()->getSR());
    for (uint8_t i = 0x11; i <= 0x1a; i++) {
      ESP_LOGI(TAG, "vic[%x] = %B", i,
               CBEServiceLocator::getCPUC64()->vic->vicreg[i]);
    }
    /*
    ESP_LOGI(TAG, "l11 = %B, l12 = %B, vicmem = %x", vic->latchd011,
             vic->latchd012, vic->vicmem);
    for (uint8_t i = 0x04; i <= 0x0f; i++) {
      ESP_LOGI(TAG, "cia1[%x] = %B", i, cia1->ciareg[i]);
    }
    ESP_LOGI(TAG,
             "l04 = %B, l05 = %B, l06 = %B, l07 = %B, l0d = %B, timerA = "
             "%x, timerB = %x, reloadA = %t, reloadB = %t",
             cia1->latchdc04, cia1->latchdc05, cia1->latchdc06, cia1->latchdc07,
             cia1->latchdc0d, cia1->timerA, cia1->timerB, cia1->reloadA,
             cia1->reloadB);
    */
    return 2;
  case ExtCmd::SHOWMEM:
    while (!CBEServiceLocator::getBLEKB()->getData(&numofbytes)) {
    }
    while (!CBEServiceLocator::getBLEKB()->getData(&low)) {
    }
    while (!CBEServiceLocator::getBLEKB()->getData(&hi)) {
    }
    addr = low + (hi << 8);
    for (uint8_t i = 0; i < numofbytes; i++) {
      ESP_LOGI(TAG, "ram[%x] = %x", addr, ram[addr]);
      addr++;
    }
    return 0;
  case ExtCmd::RESET:
    CBEServiceLocator::getCPUC64()->cpuhalted = true;
    CBEServiceLocator::getCPUC64()->initMemAndRegs();
    CBEServiceLocator::getCPUC64()->vic->initVarsAndRegs();
    CBEServiceLocator::getCPUC64()->cia1.init();
    CBEServiceLocator::getCPUC64()->cia2.init();
    CBEServiceLocator::getCPUC64()->cpuhalted = false;
    return 0;
  case ExtCmd::JOYSTICKMODE1:
    CBEServiceLocator::getCPUC64()->joystickmode = 1;
    CBEServiceLocator::getCPUC64()->kbjoystickmode = 0;
    ESP_LOGI(TAG, "joystickmode = %x",
             CBEServiceLocator::getCPUC64()->joystickmode);
    setType1Notification();
    return 1;
  case ExtCmd::JOYSTICKMODE2:
    CBEServiceLocator::getCPUC64()->joystickmode = 2;
    CBEServiceLocator::getCPUC64()->kbjoystickmode = 0;
    ESP_LOGI(TAG, "joystickmode = %x",
             CBEServiceLocator::getCPUC64()->joystickmode);
    setType1Notification();
    return 1;
  case ExtCmd::JOYSTICKMODEOFF:
    CBEServiceLocator::getCPUC64()->joystickmode = 0;
    ESP_LOGI(TAG, "joystickmode = %x",
             CBEServiceLocator::getCPUC64()->joystickmode);
    setType1Notification();
    return 1;
  case ExtCmd::KBJOYSTICKMODE1:
    CBEServiceLocator::getCPUC64()->kbjoystickmode = 1;
    CBEServiceLocator::getCPUC64()->joystickmode = 0;
    ESP_LOGI(TAG, "kbjoystickmode = %x",
             CBEServiceLocator::getCPUC64()->kbjoystickmode);
    return 1;
  case ExtCmd::KBJOYSTICKMODE2:
    CBEServiceLocator::getCPUC64()->kbjoystickmode = 2;
    CBEServiceLocator::getCPUC64()->joystickmode = 0;
    ESP_LOGI(TAG, "kbjoystickmode = %x",
             CBEServiceLocator::getCPUC64()->kbjoystickmode);
    return 1;
  case ExtCmd::KBJOYSTICKMODEOFF:
    CBEServiceLocator::getCPUC64()->kbjoystickmode = 0;
    ESP_LOGI(TAG, "kbjoystickmode = %x",
             CBEServiceLocator::getCPUC64()->kbjoystickmode);
    return 1;
  case ExtCmd::GETSTATUS:
    // just send type 1 notification
    ESP_LOGI(TAG, "send status to BLE client");
    setType1Notification();
    return 1;
  case ExtCmd::SWITCHFRAMECOLORREFRESH:
    CBEServiceLocator::getCPUC64()->refreshframecolor =
        !CBEServiceLocator::getCPUC64()->refreshframecolor;
    ESP_LOGI(TAG, "refreshframecolor = %B",
             CBEServiceLocator::getCPUC64()->refreshframecolor);
    return 0;
  }
  return 0;
}
