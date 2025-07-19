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
#include "SDCard.h"
#include "../Config.h"
#ifdef USE_SDCARD
#include "../OSUtils.h"
#include <SD_MMC.h>

static const char *TAG = "SDCard";

SDCard::SDCard() : initalized(false) {}

bool SDCard::init() {
  if (initalized) {
    return true;
  }
  vTaskDelay(500 / portTICK_PERIOD_MS);
  SD_MMC.setPins(Config::SD_SCLK_PIN, Config::SD_MOSI_PIN, Config::SD_MISO_PIN);
  bool rlst = SD_MMC.begin("/sdcard", true);
  if (!rlst) {
    return false;
  }
  initalized = true;
  return true;
}

uint16_t SDCard::load(char *path, uint8_t *ram) {
  if (!initalized) {
    return 0;
  }
  OSUtils::log(LOG_INFO, TAG, "load file %s", path);
  File file = SD_MMC.open(path);
  if (!file) {
    return 0;
  }
  uint16_t addr = 0;
  uint8_t byte = 0;
  if (file.available()) {
    byte = (uint8_t)file.read();
    addr = byte;
  }
  if (file.available()) {
    byte = (uint8_t)file.read();
    addr += byte << 8;
  }
  while (file.available()) {
    byte = (uint8_t)file.read();
    ram[addr++] = byte;
  }
  file.close();
  return addr;
}

bool SDCard::save(char *path, uint8_t *ram, uint16_t startaddr,
                  uint16_t endaddr) {
  if (!initalized) {
    return false;
  }
  OSUtils::log(LOG_INFO, TAG, "save file %s", path);
  File file = SD_MMC.open(path, FILE_WRITE);
  if (!file) {
    return false;
  }
  file.write(startaddr & 0xff);
  file.write(startaddr / 256);
  for (uint16_t i = startaddr; i < endaddr; i++) {
    file.write(ram[i]);
  }
  file.close();
  return true;
}

bool SDCard::listnextentry(uint8_t *nextentry, bool start) {
  if (!initalized) {
    return false;
  }
  File file;
  if (start) {
    if (root) {
      root.close();
    }
    root = SD_MMC.open("/");
    if (!root || !root.isDirectory()) {
      OSUtils::log(LOG_INFO, TAG, "cannot open root dir");
      return false;
    }
    file = root.openNextFile();
  } else {
    file = root.openNextFile();
  }
  if (!file) {
    root.close();
    nextentry[0] = '\0';
    return true;
  }
  String filename = file.name();
  if (filename.endsWith(".prg")) {
    filename = filename.substring(0, filename.length() - 4);
  }
  const char *fname = filename.c_str();
  for (uint8_t i = 0; i < 16; i++) {
    uint8_t p = fname[i];
    if ((p >= 96 + 1) && (p <= 96 + 26)) {
      nextentry[i] = p - 32;
    } else {
      nextentry[i] = p;
    }
  }
  nextentry[16] = '\0';
  return true;
}
#endif
