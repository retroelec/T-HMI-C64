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
#include "SDCard.h"
#include <ArduinoLog.h>

SDCard::SDCard() : initalized(false) {}

bool SDCard::init() {
  if (initalized) {
    return true;
  }
  delay(1000);
  pinMode(Config::PWR_EN_PIN, OUTPUT);
  digitalWrite(Config::PWR_EN_PIN, HIGH);
  SD_MMC.setPins(Config::SD_SCLK_PIN, Config::SD_MOSI_PIN, Config::SD_MISO_PIN);
  bool rlst = SD_MMC.begin("/sdcard", true);
  if (!rlst) {
    return false;
  }
  initalized = true;
  return true;
}

uint16_t SDCard::load(fs::FS &fs, uint8_t *cursorpos, uint8_t *ram) {
  if (!initalized) {
    return 0;
  }
  cursorpos--; // char may be 160
  while (*cursorpos == 32) {
    cursorpos--;
  }
  while (*cursorpos != 32) {
    cursorpos--;
  }
  cursorpos++;
  char path[22];
  path[0] = '/';
  uint8_t i = 1;
  uint8_t p;
  while (((p = *cursorpos++) != 32) && (p != 160) && (i < 16)) {
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
  Log.noticeln("load file %s", path);
  File file = fs.open(path);
  if (!file) {
    return 0;
  }
  uint16_t chksum = 0;
  uint16_t addr = 0;
  uint8_t byte = 0;
  if (file.available()) {
    byte = (uint8_t)file.read();
    addr = byte;
    chksum += byte;
  }
  if (file.available()) {
    byte = (uint8_t)file.read();
    addr += byte << 8;
    chksum += byte;
  }
  while (file.available()) {
    byte = (uint8_t)file.read();
    ram[addr++] = byte;
    chksum += byte;
  }
  Log.noticeln("chksum = %d", chksum);
  return addr;
}
