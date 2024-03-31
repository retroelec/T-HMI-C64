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
#ifndef ST7789V_H
#define ST7789V_H

#include <cstdint>

class ST7789V {
private:
  inline static void writeCmd(uint8_t cmd) __attribute__((always_inline));
  inline static void writeData(uint8_t data) __attribute__((always_inline));
  static void copyData(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h,
                       uint16_t *data);
  static uint16_t *framecolormem;

public:
  static const uint16_t c64_black = 0x0000;
  static const uint16_t c64_white = 0xffff;
  static const uint16_t c64_red = 0x8000;
  static const uint16_t c64_turquoise = 0xa7fc;
  static const uint16_t c64_purple = 0xc218;
  static const uint16_t c64_green = 0x064a;
  static const uint16_t c64_blue = 0x0014;
  static const uint16_t c64_yellow = 0xe74e;
  static const uint16_t c64_orange = 0xd42a;
  static const uint16_t c64_brown = 0x6200;
  static const uint16_t c64_lightred = 0xfbae;
  static const uint16_t c64_grey1 = 0x3186;
  static const uint16_t c64_grey2 = 0x73ae;
  static const uint16_t c64_lightgreen = 0xa7ec;
  static const uint16_t c64_lightblue = 0x043f;
  static const uint16_t c64_grey3 = 0xb5d6;

  static bool init();
  static void drawFrame(uint16_t frameColor);
  static void drawBitmap(uint16_t *bitmap);
};

#endif // ST7789V_H
