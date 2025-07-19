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
#ifndef FSFACTORY_H
#define FSFACTORY_H

#include "../Config.h"
#include "FSDriver.h"
#if defined(USE_SDCARD)
#include "SDCard.h"
#elif defined(USE_NOFS)
#include "NoFS.h"
#else
#error "no valid fs driver defined"
#endif

namespace FileSys {
FSDriver *create() {
#if defined(USE_SDCARD)
  return new SDCard();
#elif defined(USE_NOFS)
  return new NoFS();
#endif
}
} // namespace FileSys

#endif // FSFACTORY_H
