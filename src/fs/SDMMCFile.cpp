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
#include "SDMMCFile.h"
#include "../Config.h"
#ifdef USE_SDCARD
#include "../platform/PlatformManager.h"
#include <SD_MMC.h>

static const char *TAG = "SDMMCFile";

bool SDMMCFile::initialized = false;

bool SDMMCFile::init() {
  if (SDMMCFile::initialized) {
    return true;
  }
  PlatformManager::getInstance().waitMS(500);
  SD_MMC.setPins(Config::SD_SCLK_PIN, Config::SD_MOSI_PIN, Config::SD_MISO_PIN);
  bool rlst = SD_MMC.begin("/sdcard", true);
  if (!rlst) {
    return false;
  }
  SDMMCFile::initialized = true;
  return true;
}

bool SDMMCFile::open(const std::string &path, const char *mode) {
  close();
  const char *m = (mode[0] == 'r') ? FILE_READ : FILE_WRITE;
  std::string path1 = '/' + path;
  file = SD_MMC.open(path1.c_str(), m);
  return file;
}

size_t SDMMCFile::read(void *buffer, size_t count) {
  return file ? file.read((uint8_t *)buffer, count) : 0;
}

size_t SDMMCFile::write(const void *buffer, size_t count) {
  return file ? file.write((const uint8_t *)buffer, count) : 0;
}

bool SDMMCFile::seek(long offset, int origin) {
  if (!file) {
    return false;
  }
  if (origin == SEEK_SET) {
    return file.seek(offset);
  }
  if (origin == SEEK_CUR) {
    return file.seek(file.position() + offset);
  }
  if (origin == SEEK_END) {
    return file.seek(file.size() + offset);
  }
  return false;
}

long SDMMCFile::tell() const { return file ? file.position() : -1; }

bool SDMMCFile::eof() { return file && (!file.available()); }

int64_t SDMMCFile::size() {
  return file ? static_cast<int64_t>(file.size()) : -1;
}

void SDMMCFile::close() {
  if (file) {
    file.close();
  }
}

File listroot;

bool SDMMCFile::listnextentry(std::string &name, bool start) {
  name = "";
  if (!SDMMCFile::initialized) {
    return false;
  }
  File file;
  if (start) {
    if (listroot) {
      listroot.close();
    }
    listroot = SD_MMC.open("/");
    if (!listroot || !listroot.isDirectory()) {
      PlatformManager::getInstance().log(LOG_INFO, TAG, "cannot open root dir");
      return false;
    }
    file = listroot.openNextFile();
  } else {
    file = listroot.openNextFile();
  }
  if (!file) {
    listroot.close();
    return true;
  }
  name = file.name();
  return true;
}

SDMMCFile::~SDMMCFile() { close(); }
#endif
