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
#include "../Config.h"
#ifdef USE_ST7789VSERIAL
// files Display_ST7789.cpp and Display_ST7789.h are copied from
// https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.8/ESP32-S3-Touch-LCD-2.8-Demo.zip
// and slightly adjusted
#include "ST7789VSerial.h"
#include "st7789vserial/Display_ST7789.h"

uint16_t *ST7789VSerial::framecolormem;

void ST7789VSerial::init() {
  ST7789VSerial::framecolormem = new uint16_t[FRAMEMEMSIZE]();
  LCD_Init();
  Backlight_Init();
  oldFrameColor = 0;
}

void ST7789VSerial::drawFrame(uint16_t frameColor) {
  if (frameColor == oldFrameColor) {
    return;
  }
  oldFrameColor = frameColor;
  uint16_t cnt = FRAMEMEMSIZE;
  uint16_t *frameptr = ST7789VSerial::framecolormem;
  while (cnt--) {
    *frameptr = frameColor;
    frameptr++;
  }
  if (BORDERHEIGHT > 0) {
    LCD_addWindow(0, 0, Config::LCDWIDTH - 1, BORDERHEIGHT - 1,
                  ST7789VSerial::framecolormem);
    LCD_addWindow(0, 200 + BORDERHEIGHT, Config::LCDWIDTH - 1,
                  Config::LCDHEIGHT - 1, ST7789VSerial::framecolormem);
  }
  if (BORDERWIDTH > 0) {
    LCD_addWindow(0, BORDERHEIGHT, BORDERWIDTH - 1, BORDERHEIGHT + 200 - 1,
                  ST7789VSerial::framecolormem);
    LCD_addWindow(BORDERWIDTH + 320, BORDERHEIGHT, Config::LCDWIDTH - 1,
                  BORDERHEIGHT + 200 - 1, ST7789VSerial::framecolormem);
  }
}

void LCD_addWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend,
                   uint16_t Yend, uint16_t *color);

void ST7789VSerial::drawBitmap(uint16_t *bitmap) {
  LCD_addWindow(BORDERWIDTH, BORDERHEIGHT, 319 + BORDERWIDTH,
                199 + BORDERHEIGHT, bitmap);
}

#endif
