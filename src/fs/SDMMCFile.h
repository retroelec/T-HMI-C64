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
#ifndef SDMMCFILE_H
#define SDMMCFILE_H

#include "../Config.h"
#ifdef USE_SDCARD
#include "FileDriver.h"
#include <FS.h>
#include <cstdint>

class SDMMCFile : public FileDriver {
private:
  static bool initialized;
  File file;

public:
  bool init() override;
  bool open(const std::string &path, const char *mode) override;
  size_t read(void *buffer, size_t count) override;
  size_t write(const void *buffer, size_t count) override;
  bool seek(long offset, int origin) override;
  long tell() const override;
  bool eof() override;
  int64_t size() override;
  void close() override;
  bool listnextentry(std::string &name, bool start) override;
  ~SDMMCFile();
};
#endif

#endif // SDMMCFILE_H
