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
#include "Config.h"
#include <esp_log.h>

static const char *TAG = "SDCard";

SDCard::SDCard() : initalized(false) {}

bool SDCard::init() {
#if defined(USE_SDCARD)
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
#else
  return false;
#endif
}

void getPath(char *path, uint8_t *ram) {
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
  path[0] = '/';
  uint8_t i = 1;
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

uint16_t SDCard::load(fs::FS &fs, uint8_t *ram) {
  if (!initalized) {
    return 0;
  }
  char path[22];
  getPath(path, ram);
  ESP_LOGI(TAG, "load file %s", path);
  File file = fs.open(path);
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

bool SDCard::save(fs::FS &fs, uint8_t *ram) {
  if (!initalized) {
    return false;
  }
  char path[22];
  getPath(path, ram);
  uint16_t startaddr = ram[43] + ram[44] * 256;
  uint16_t endaddr = ram[45] + ram[46] * 256;
  ESP_LOGI(TAG, "save file %s", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    return false;
  }
  file.write(ram[43]);
  file.write(ram[44]);
  for (uint16_t i = startaddr; i < endaddr; i++) {
    file.write(ram[i]);
  }
  file.close();
  return true;
}

bool SDCard::listnextentry(fs::FS &fs, uint8_t *nextentry, bool start) {
  if (!initalized) {
    return false;
  }
  File file;
  if (start) {
    if (root) {
      root.close();
    }
    root = fs.open("/");
    if (!root || !root.isDirectory()) {
      ESP_LOGI(TAG, "cannot open root dir");
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
