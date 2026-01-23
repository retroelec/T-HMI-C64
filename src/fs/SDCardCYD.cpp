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
#include "SDCardCYD.h"
#include "../Config.h"
#ifdef USE_SDCARDCYD
#include "../platform/PlatformManager.h"
#include <SD.h>
#include <SPI.h>

static const char *TAG = "SDCardCYD";

bool SDCardCYD::initialized = false;

bool SDCardCYD::init() {
  if (SDCardCYD::initialized) {
    return true;
  }
  PlatformManager::getInstance().waitMS(500);
  SPI.begin(Config::SD_SCLK_PIN, Config::SD_MISO_PIN, Config::SD_MOSI_PIN,
            Config::SD_CS_PIN);
  bool rlst = SD.begin(Config::SD_CS_PIN);
  if (!rlst) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "SD Card mount failed!");
    return false;
  }
  SDCardCYD::initialized = true;
  return true;
}

bool SDCardCYD::open(const std::string &path, const char *mode) {
  PlatformLock lock(PlatformManager::getInstance());
  close();
  const char *m = (mode[0] == 'r') ? FILE_READ : FILE_WRITE;
  std::string path1 = (path[0] == '/') ? path : '/' + path;
  file = SD.open(path1.c_str(), m);
  return file;
}

size_t SDCardCYD::read(void *buffer, size_t count) {
  PlatformLock lock(PlatformManager::getInstance());
  return file ? file.read((uint8_t *)buffer, count) : 0;
}

size_t SDCardCYD::write(const void *buffer, size_t count) {
  PlatformLock lock(PlatformManager::getInstance());
  return file ? file.write((const uint8_t *)buffer, count) : 0;
}

bool SDCardCYD::seek(long offset, int origin) {
  if (!file)
    return false;
  PlatformLock lock(PlatformManager::getInstance());
  if (origin == SEEK_SET)
    return file.seek(offset);
  if (origin == SEEK_CUR)
    return file.seek(file.position() + offset);
  if (origin == SEEK_END)
    return file.seek(file.size() + offset);
  return false;
}

long SDCardCYD::tell() const { return file ? file.position() : -1; }

bool SDCardCYD::eof() { return file && (!file.available()); }

int64_t SDCardCYD::size() {
  return file ? static_cast<int64_t>(file.size()) : -1;
}

void SDCardCYD::close() {
  if (file) {
    file.close();
  }
}

File listroot;

bool SDCardCYD::listnextentry(std::string &name, bool start) {
  name = "";
  if (!SDCardCYD::initialized) {
    return false;
  }
  PlatformLock lock(PlatformManager::getInstance());
  if (start) {
    if (listroot) {
      listroot.close();
    }
    listroot = SD.open("/");
    if (!listroot || !listroot.isDirectory()) {
      PlatformManager::getInstance().log(LOG_INFO, TAG, "cannot open root dir");
      return false;
    }
  }
  File entry = listroot.openNextFile();
  if (!entry) {
    listroot.close();
    return true;
  }
  name = entry.name();
  entry.close();
  return true;
}

SDCardCYD::~SDCardCYD() { close(); }
#endif
