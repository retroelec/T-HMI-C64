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
#ifndef T_DISPLAY_S3_H
#define T_DISPLAY_S3_H

#include "../Config.h"
#ifdef BOARD_T_DISPLAY_S3
#include "BoardDriver.h"

class T_DISPLAY_S3 : public BoardDriver {
public:
  void init() override {}

  uint16_t getBatteryVoltage() override { return 4000; }

  void powerOff() override {}
};
#endif

#endif // T_DISPLAY_S3_H
