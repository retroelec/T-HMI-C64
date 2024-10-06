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
#ifndef CONFIGDISPLAY_H
#define CONFIGDISPLAY_H

#include "DisplayDriver.h"
#include "ST7789V.h"
//#include "RM67162.h"

struct ConfigDisplay {
  DisplayDriver *displayDriver;
  ConfigDisplay() { displayDriver = new ST7789V(); }
  // ConfigDisplay() { displayDriver = new RM67162(); }
};

#endif // CONFIGDISPLAY_H
