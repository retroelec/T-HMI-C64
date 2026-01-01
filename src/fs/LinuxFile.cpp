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
#include "../Config.h"
#ifdef USE_LINUXFS
#include "LinuxFile.h"
#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <iostream>
#include <string>
#include <sys/types.h>

LinuxFile::~LinuxFile() { close(); }

bool LinuxFile::open(const std::string &path, const char *mode) {
  close();
  fp = std::fopen(path.c_str(), mode);
  return fp != nullptr;
}

size_t LinuxFile::read(void *buffer, size_t count) {
  return fp ? std::fread(buffer, 1, count, fp) : 0;
}

size_t LinuxFile::write(const void *buffer, size_t count) {
  return fp ? std::fwrite(buffer, 1, count, fp) : 0;
}

bool LinuxFile::seek(long offset, int origin) {
  return fp && std::fseek(fp, offset, origin) == 0;
}

long LinuxFile::tell() const { return fp ? std::ftell(fp) : -1; }

bool LinuxFile::eof() { return fp && std::feof(fp); }

int64_t LinuxFile::size() {
  if (!fp) {
    return -1;
  }
  std::fseek(fp, 0, SEEK_END);
  int64_t filesize = std::ftell(fp);
  std::fseek(fp, 0, SEEK_SET);
  return filesize;
}

void LinuxFile::close() {
  if (fp) {
    std::fclose(fp);
    fp = nullptr;
  }
}

static DIR *dir_stream = nullptr;

bool LinuxFile::listnextentry(std::string &name, bool start) {
  name = "";
  if (start) {
    if (dir_stream != nullptr) {
      closedir(dir_stream);
      dir_stream = nullptr;
    }
    dir_stream = opendir(Config::PATH);
    if (dir_stream == nullptr) {
      return false;
    }
  }
  errno = 0;
  struct dirent *entry = readdir(dir_stream);
  if (entry == nullptr) {
    if (errno != 0) {
      closedir(dir_stream);
      dir_stream = nullptr;
      return false;
    }
    // end of directory reached
    closedir(dir_stream);
    dir_stream = nullptr;
    return true;
  }
  std::string filename(entry->d_name);
  // ignore . and ..
  if (filename == "." || filename == "..") {
    // call method recursively to get next entry
    return listnextentry(name, false);
  }
  name = filename;
  return true;
}

#endif
