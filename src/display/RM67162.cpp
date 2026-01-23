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
#include "RM67162.h"
// files rm67162.cpp, rm67162.h and pins_config.h are copied from
// https://github.com/Xinyuan-LilyGO/T-Display-S3-AMOLED/tree/main/examples/factory
#include "../Config.h"
#ifdef USE_RM67162
#include "BitmapUtils.h"
#include "rm67162/rm67162.h"

static constexpr uint16_t BORDERWIDTH = (Config::LCDWIDTH - 320) / 2;
static constexpr uint16_t BORDERHEIGHT = (Config::LCDHEIGHT - 200) / 2;
static constexpr uint8_t BUFNUMLINES = 20;
static constexpr uint16_t DBBUFSIZE = 320 * BUFNUMLINES;
static constexpr uint16_t DFBUFSIZE =
    MAX(320 * BORDERHEIGHT, BORDERWIDTH *Config::LCDHEIGHT);
static constexpr uint16_t BUFFERSIZE = MAX(DFBUFSIZE, DBBUFSIZE);

static uint16_t *buffer __attribute__((aligned(4)));

void RM67162::init() {
  rm67162_init();
  lcd_setRotation(1);
  buffer = new uint16_t[BUFFERSIZE];
  oldFrameColor = 0;
}

void RM67162::drawFrame(uint8_t frameColor) {
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
    lcd_PushColors(BORDERWIDTH, 0, 320, BORDERHEIGHT, buffer);
    lcd_PushColors(BORDERWIDTH, 200 + BORDERHEIGHT, 320, BORDERHEIGHT, buffer);
  }
  if (BORDERWIDTH > 0) {
    lcd_PushColors(0, 0, BORDERWIDTH, Config::LCDHEIGHT, buffer);
    lcd_PushColors(BORDERWIDTH + 320, 0, BORDERWIDTH, Config::LCDHEIGHT,
                   buffer);
  }
}

void RM67162::drawBitmap(uint8_t *bitmap) {
  uint16_t ystart = BORDERHEIGHT;
  for (uint8_t i = 0; i < 200 / BUFNUMLINES; i++) {
    BitmapUtils::getBitmap(bitmap + (i * DBBUFSIZE), buffer, c64Colors,
                           DBBUFSIZE);
    lcd_PushColors(BORDERWIDTH, ystart, 320, BUFNUMLINES, buffer);
    ystart += BUFNUMLINES;
  }
}

#endif
