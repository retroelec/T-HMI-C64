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
#ifndef ST7789VSERIAL_H
#define ST7789VSERIAL_H

#include "../Config.h"
#ifdef USE_ST7789VSERIAL
#include "DisplayDriver.h"
#include <cstdint>

// no elegant/simple solution for max() at compile time in C++11
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

class ST7789VSerial : public DisplayDriver {
private:
  static constexpr uint16_t BORDERWIDTH = (Config::LCDWIDTH - 320) / 2;
  static constexpr uint16_t BORDERHEIGHT = (Config::LCDHEIGHT - 200) / 2;
  static constexpr uint16_t FRAMEMEMSIZE =
      MAX(BORDERHEIGHT * Config::LCDWIDTH, BORDERWIDTH * 200);
  static uint16_t *framecolormem;
  uint16_t oldFrameColor;

public:
  void init() override;
  void drawFrame(uint16_t frameColor) override;
  void drawBitmap(uint16_t *bitmap) override;
};
#endif

#endif // ST7789VSERIAL_H
