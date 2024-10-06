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
#ifndef DISPLAYDRIVER_H
#define DISPLAYDRIVER_H

#include <cstdint>

class DisplayDriver {
public:
  virtual void init() = 0;
  virtual void drawFrame(uint16_t frameColor) = 0;
  virtual void drawBitmap(uint16_t *bitmap) = 0;
  virtual const uint16_t *getC64Colors() const = 0;
  virtual ~DisplayDriver() {}
};

#endif // DISPLAYDRIVER_H
