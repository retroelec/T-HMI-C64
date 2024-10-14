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
#ifndef RM67162_H
#define RM67162_H

#include "Config.h"
#include "DisplayDriver.h"
#include <cstdint>

// no elegant/simple solution for max() at compile time in C++11
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

class RM67162 : public DisplayDriver {
private:
  static const uint16_t c64_black = 0x0000;
  static const uint16_t c64_white = 0xffff;
  static const uint16_t c64_red = 0x0088;
  static const uint16_t c64_turquoise = 0xfca7;
  static const uint16_t c64_purple = 0x18c2;
  static const uint16_t c64_green = 0x4a06;
  static const uint16_t c64_blue = 0x1400;
  static const uint16_t c64_yellow = 0x4ee7;
  static const uint16_t c64_orange = 0x2ad4;
  static const uint16_t c64_brown = 0x0062;
  static const uint16_t c64_lightred = 0xaefb;
  static const uint16_t c64_grey1 = 0x8631;
  static const uint16_t c64_grey2 = 0xae73;
  static const uint16_t c64_lightgreen = 0xeca7;
  static const uint16_t c64_lightblue = 0x3f04;
  static const uint16_t c64_grey3 = 0xd6b5;

  const uint16_t c64Colors[16] = {
      c64_black,  c64_white,      c64_red,       c64_turquoise,
      c64_purple, c64_green,      c64_blue,      c64_yellow,
      c64_orange, c64_brown,      c64_lightred,  c64_grey1,
      c64_grey2,  c64_lightgreen, c64_lightblue, c64_grey3};

  static const uint16_t BORDERWIDTH = (Config::LCDWIDTH - 320) / 2;
  static const uint16_t BORDERHEIGHT = (Config::LCDHEIGHT - 200) / 2;
  static const uint16_t FRAMEMEMSIZE =
      MAX(320 * BORDERHEIGHT, BORDERWIDTH *Config::LCDHEIGHT);
  static uint16_t *framecolormem;

public:
  void init() override;
  void drawFrame(uint16_t frameColor) override;
  void drawBitmap(uint16_t *bitmap) override;
  const uint16_t *getC64Colors() const override;
};

#endif // RM67162_H
