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
#include "RM67162.h"
// files rm67162.cpp, rm67162.h and pins_config.h are copied from
// https://github.com/Xinyuan-LilyGO/T-Display-S3-AMOLED/tree/main/examples/factory
#include "Config.h"
#ifdef USE_RM67162
#include "rm67162/rm67162.h"

uint16_t *RM67162::framecolormem;

void RM67162::init() {
  RM67162::framecolormem = new uint16_t[FRAMEMEMSIZE]();
  rm67162_init();
  lcd_setRotation(1);
}

void RM67162::drawFrame(uint16_t frameColor) {
  uint16_t cnt = FRAMEMEMSIZE;
  uint16_t *frameptr = RM67162::framecolormem;
  while (cnt--) {
    *frameptr = frameColor;
    frameptr++;
  }
  if (BORDERHEIGHT > 0) {
    lcd_PushColors(BORDERWIDTH, 0, 320, BORDERHEIGHT, RM67162::framecolormem);
    lcd_PushColors(BORDERWIDTH, 200 + BORDERHEIGHT, 320, BORDERHEIGHT,
                   RM67162::framecolormem);
  }
  if (BORDERWIDTH > 0) {
    lcd_PushColors(0, 0, BORDERWIDTH, Config::LCDHEIGHT,
                   RM67162::framecolormem);
    lcd_PushColors(BORDERWIDTH + 320, 0, BORDERWIDTH, Config::LCDHEIGHT,
                   RM67162::framecolormem);
  }
}

void RM67162::drawBitmap(uint16_t *bitmap) {
  lcd_PushColors(BORDERWIDTH, BORDERHEIGHT, 320, 200, bitmap);
}

const uint16_t *RM67162::getC64Colors() const { return c64Colors; }
#endif
