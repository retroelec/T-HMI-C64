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
#ifndef ST7789V_H
#define ST7789V_H

#include "../Config.h"
#ifdef USE_ST7789V
#include "DisplayDriver.h"
#include <cstdint>

// no elegant/simple solution for max() at compile time in C++11
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

class ST7789V : public DisplayDriver {
private:
  static const uint16_t BORDERWIDTH = (Config::LCDWIDTH - 320) / 2;
  static const uint16_t BORDERHEIGHT = (Config::LCDHEIGHT - 200) / 2;
  static const uint16_t FRAMEMEMSIZE =
      MAX(320 * BORDERHEIGHT, BORDERWIDTH *Config::LCDHEIGHT);

  uint16_t oldFrameColor;

  inline static void writeCmd(uint8_t cmd) __attribute__((always_inline));
  inline static void writeData(uint8_t data) __attribute__((always_inline));
  inline static void copyinit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h)
      __attribute__((always_inline));
  inline static void copycopy(uint16_t data, uint32_t clearMask)
      __attribute__((always_inline));
  inline static void copyend() __attribute__((always_inline));
  static void copyColor(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h,
                        uint16_t data);
  static void copyData(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h,
                       uint16_t *data);

public:
  void init() override;
  void drawFrame(uint16_t frameColor) override;
  void drawBitmap(uint16_t *bitmap) override;
};
#endif

#endif // ST7789V_H
