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
#ifndef BOARDDRIVER_H
#define BOARDDRIVER_H

class BoardDriver {
public:
  virtual void init() = 0;
  virtual void powerOff() = 0;
  virtual adc_oneshot_unit_handle_t getAdcHandle() = 0;
  virtual adc_cali_handle_t getAdcCaliHandle() = 0;
  virtual ~BoardDriver() {}
};

#endif // BOARDDRIVER_H
