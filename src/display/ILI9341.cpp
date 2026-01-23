/*
 Copyright (C) 2024-2026 retroelec <retroelec42@gmail.com>

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
#ifdef USE_ILI9341
#include "BitmapUtils.h"
#include "ILI9341.h"
#include "ili9341/lcd.h"

static constexpr uint16_t BORDERWIDTH = (Config::LCDWIDTH - 320) / 2;
static constexpr uint16_t BORDERHEIGHT = (Config::LCDHEIGHT - 200) / 2;
static constexpr uint8_t BUFNUMLINES = 16;
static constexpr uint16_t DBBUFSIZE = 320 * BUFNUMLINES;
static constexpr uint16_t DFBUFSIZE =
    MAX(320 * BORDERHEIGHT, BORDERWIDTH *Config::LCDHEIGHT);
static constexpr uint16_t BUFFERSIZE = MAX(DFBUFSIZE, DBBUFSIZE);

static uint16_t *buffer __attribute__((aligned(4)));

void ILI9341::init() {
  Init_LCD();
  LCD_Set_Orientation(LCD_DISPLAY_ORIENTATION_LANDSCAPE);
  buffer = new uint16_t[BUFFERSIZE];
  oldFrameColor = 0;
}

void ILI9341::drawFrame(uint8_t frameColor) {
  if (frameColor == oldFrameColor) {
    return;
  }
  oldFrameColor = frameColor;
  uint16_t cnt = DFBUFSIZE;
  uint16_t *frameptr = buffer;
  uint16_t frameColor16 = c64Colors[frameColor];
  while (cnt--) {
    *frameptr++ = frameColor16;
  }
  if (BORDERHEIGHT > 0) {
    LCD_SetWindows(0, 0, Config::LCDWIDTH - 1, BORDERHEIGHT - 1);
    LCD_WriteDate(buffer, DBBUFSIZE * 2);
    LCD_WriteDate(buffer, Config::LCDWIDTH * 4 * 2);
    LCD_SetWindows(0, 200 + BORDERHEIGHT, Config::LCDWIDTH - 1,
                   Config::LCDHEIGHT - 1);
    LCD_WriteDate(buffer, DBBUFSIZE * 2);
    LCD_WriteDate(buffer, Config::LCDWIDTH * 4 * 2);
  }
}

void ILI9341::drawBitmap(uint8_t *bitmap) {
  uint16_t xstart = BORDERWIDTH;
  uint16_t ystart = BORDERHEIGHT;
  uint16_t xend = 319 + BORDERWIDTH;
  uint16_t yend = 199 + BORDERHEIGHT;
  LCD_SetWindows(xstart, ystart, xend, yend + BORDERHEIGHT);
  for (uint8_t i = 0; i < 12; i++) {
    BitmapUtils::getBitmap(bitmap + (i * DBBUFSIZE), buffer, c64Colors,
                           DBBUFSIZE);
    LCD_WriteDate(buffer, DBBUFSIZE * 2);
  }
  BitmapUtils::getBitmap(bitmap + (12 * DBBUFSIZE), buffer, c64Colors,
                         DBBUFSIZE / 2);
  LCD_WriteDate(buffer, DBBUFSIZE);
}

void ILI9341::reconfigureSPI() { LCD_Restore_Interface(); }

#endif
