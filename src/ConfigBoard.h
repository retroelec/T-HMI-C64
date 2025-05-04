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
#ifndef CONFIGBOARD_H
#define CONFIGBOARD_H

#include "BoardDriver.h"
#include "Config.h"
#if defined(BOARD_T_HMI)
#include "T_HMI.h"
#elif defined(BOARD_T_DISPLAY_S3)
#include "T_DISPLAY_S3.h"
#elif defined(BOARD_WAVESHARE)
#include "Waveshare.h"
#endif

struct ConfigBoard {
  BoardDriver *boardDriver;
#if defined(BOARD_T_HMI)
  ConfigBoard() { boardDriver = new T_HMI(); }
#elif defined(BOARD_T_DISPLAY_S3)
  ConfigBoard() { boardDriver = new T_DISPLAY_S3(); }
#elif defined(BOARD_WAVESHARE)
  ConfigBoard() { boardDriver = new Waveshare(); }
#endif
};

#endif // CONFIGBOARD_H
