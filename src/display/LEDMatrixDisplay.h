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
#ifndef LEDMATRIXDISPLAY_H
#define LEDMATRIXDISPLAY_H

#include "../Config.h"
#ifdef USE_LEDMATRIXDISPLAY
#include "DisplayDriver.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <cstdint>

class LEDMatrixDisplay : public DisplayDriver {
private:
  static const uint16_t c64_brown = 0x9100;

  const uint16_t c64Colors[16] = {
      c64_black,  c64_white,      c64_red,       c64_turquoise,
      c64_purple, c64_green,      c64_blue,      c64_yellow,
      c64_orange, c64_brown,      c64_lightred,  c64_grey1,
      c64_grey2,  c64_lightgreen, c64_lightblue, c64_grey3};

  MatrixPanel_I2S_DMA *dma_display = nullptr;
  uint8_t mode = 0;
  uint8_t spritenr = 0;
  uint16_t bitmaptargetcol[Config::LEDMATRIXWIDTH * Config::LEDMATRIXHEIGHT];
  int16_t oldstartx = 0;
  int16_t oldstarty = 0;

  void extractEachXthPixel(const uint8_t *__restrict source);
  void mergeXPixels(const uint8_t *__restrict source);
  void pixelAreaFollowsSprite(const uint8_t *__restrict bitmap,
                              const uint8_t *__restrict vicreg, bool slowmove);

public:
  void init() override;
  void drawFrame(uint8_t frameColor) override;
  void drawBitmap(const uint8_t *bitmap, const uint8_t *vicreg) override;
  void setSpecial1() override;
  void setSpecial2() override;
};
#endif
#endif // LEDMATRIXDISPLAY_H
