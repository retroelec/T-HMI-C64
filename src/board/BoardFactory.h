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
#ifndef BOARDFACTORY_H
#define BOARDFACTORY_H

#include "../Config.h"
#include "BoardDriver.h"
#if defined(BOARD_T_HMI)
#include "T_HMI.h"
#elif defined(BOARD_T_DISPLAY_S3)
#include "T_DISPLAY_S3.h"
#elif defined(BOARD_WAVESHARE)
#include "Waveshare.h"
#elif defined(BOARD_LINUX)
#include "Linux.h"
#else
#error "no valid board defined"
#endif

namespace Board {
BoardDriver *create() {
#if defined(BOARD_T_HMI)
  return new T_HMI();
#elif defined(BOARD_T_DISPLAY_S3)
  return new T_DISPLAY_S3();
#elif defined(BOARD_WAVESHARE)
  return new Waveshare();
#elif defined(BOARD_LINUX)
  return new Linux();
#endif
}
} // namespace Board

#endif // BOARDFACTORY_H
