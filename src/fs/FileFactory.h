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
#ifndef FILEFACTORY_H
#define FILEFACTORY_H

#include "../Config.h"
#include "FileDriver.h"
#include <memory>

#if defined(USE_SDCARD)
#include "SDMMCFile.h"
#elif defined(USE_LINUXFS)
#include "LinuxFile.h"
#elif defined(USE_NOFS)
#include "NoFile.h"
#else
#error "no valid fs driver defined"
#endif

namespace FileSys {
std::unique_ptr<FileDriver> create() {
#if defined(USE_SDCARD)
  return std::make_unique<SDMMCFile>();
#elif defined(USE_LINUXFS)
  return std::make_unique<LinuxFile>();
#elif defined(USE_NOFS)
  return std::make_unique<NoFile>();
#endif
}
} // namespace FileSys

#endif // FILEFACTORY_H
