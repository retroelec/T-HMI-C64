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
#ifndef T_DISPLAY_S3_H
#define T_DISPLAY_S3_H

#include "BoardDriver.h"

class T_DISPLAY_S3 : public BoardDriver {
public:
  void init() override {}

  void powerOff() override {}

  adc_oneshot_unit_handle_t getAdcHandle() override { return nullptr; }

  adc_cali_handle_t getAdcCaliHandle() override { return nullptr; }
};

#endif // T_DISPLAY_S3_H
