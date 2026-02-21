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
#ifdef USE_LEDMATRIXDISPLAY
#include "../ExtCmd.h"
#include "../ExtCmdQueue.h"
#include "../platform/PlatformManager.h"
#include "LEDMatrixDisplay.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <cstdint>

void LEDMatrixDisplay::init() {
  HUB75_I2S_CFG mxconfig(Config::LEDMATRIXWIDTH / Config::NUMPANELS,
                         Config::LEDMATRIXHEIGHT / Config::NUMPANELS,
                         Config::NUMPANELS);
  mxconfig.gpio.r1 = Config::R1;
  mxconfig.gpio.g1 = Config::G1;
  mxconfig.gpio.b1 = Config::B1;
  mxconfig.gpio.r2 = Config::R2;
  mxconfig.gpio.g2 = Config::G2;
  mxconfig.gpio.b2 = Config::B2;
  mxconfig.gpio.a = Config::CH_A;
  mxconfig.gpio.b = Config::CH_B;
  mxconfig.gpio.c = Config::CH_C;
  mxconfig.gpio.d = Config::CH_D;
  mxconfig.gpio.e = Config::CH_E;
  mxconfig.gpio.lat = Config::LAT;
  mxconfig.gpio.oe = Config::OE;
  mxconfig.gpio.clk = Config::CLK;
  mxconfig.clkphase = false;
  mxconfig.setPixelColorDepthBits(4);
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  if (!dma_display->begin()) {
    while (1)
      ;
  }
  dma_display->setBrightness8(Config::LEDMATRIXBRIGHTNESS);
  dma_display->clearScreen();
}

void LEDMatrixDisplay::drawFrame(uint8_t frameColor) {}

void IRAM_ATTR
LEDMatrixDisplay::extractEachXthPixel(const uint8_t *__restrict source) {
  for (uint16_t y = 0; y < Config::LEDMATRIXHEIGHT; y++) {
    // 4 pixel offset, each 3. pixel
    const uint8_t *srcRow = &source[(4 + y * 3) * Config::C64WIDTH];
    uint16_t *dstRow = &bitmaptargetcol[y * Config::LEDMATRIXWIDTH];
    for (uint16_t x = 0; x < Config::LEDMATRIXWIDTH; x++) {
      // each 4. pixel
      uint8_t rawVal = srcRow[32 + (x * 4)];
      dstRow[x] = c64Colors[rawVal & 0x0f];
    }
  }
}

void IRAM_ATTR
LEDMatrixDisplay::mergeXPixels(const uint8_t *__restrict source) {
  for (uint16_t y = 0; y < Config::LEDMATRIXHEIGHT; y++) {
    for (uint16_t x = 0; x < Config::LEDMATRIXWIDTH; x++) {
      uint32_t sumR = 0, sumG = 0, sumB = 0;
      const uint16_t numPixels =
          Config::LEDMATRIXSCALEX * Config::LEDMATRIXSCALEY;
      for (uint16_t sy = 0; sy < Config::LEDMATRIXSCALEY; sy++) {
        uint16_t srcY = Config::IGNBORDERY + (y * Config::LEDMATRIXSCALEY) + sy;
        for (uint16_t sx = 0; sx < Config::LEDMATRIXSCALEX; sx++) {
          uint16_t srcX =
              Config::IGNBORDERX + (x * Config::LEDMATRIXSCALEX) + sx;
          uint8_t colorIdx = source[srcY * Config::C64WIDTH + srcX] & 0x0F;
          uint16_t color = c64Colors[colorIdx];
          sumR += (uint32_t)((color >> 11) & 0x1F);
          sumG += (uint32_t)((color >> 5) & 0x3F);
          sumB += (uint32_t)(color & 0x1F);
        }
      }
      uint32_t avgR = sumR / numPixels;
      uint32_t avgG = sumG / numPixels;
      uint32_t avgB = sumB / numPixels;
      bitmaptargetcol[y * Config::LEDMATRIXWIDTH + x] =
          (uint16_t)(((avgR & 0x1F) << 11) | ((avgG & 0x3F) << 5) |
                     (avgB & 0x1F));
    }
  }
}

void LEDMatrixDisplay::pixelAreaFollowsSprite(const uint8_t *__restrict bitmap,
                                              const uint8_t *__restrict vicreg,
                                              bool slowmove) {
  // sprite active?
  int16_t startx = 0;
  int16_t starty = 0;
  uint8_t spriteval = 1 << spritenr;
  if (spriteval & vicreg[0x15]) {
    int16_t startx8 = (vicreg[0x10] & spriteval) ? 256 : 0;
    startx = vicreg[spritenr * 2] + startx8 - 20 - 24;
    starty = vicreg[spritenr * 2 + 1] - 21 - 51;
    if (startx < 0) {
      startx = 0;
    } else if (startx > 320 - Config::LEDMATRIXWIDTH) {
      startx = 320 - Config::LEDMATRIXWIDTH;
    }
    if (starty < 0) {
      starty = 0;
    } else if (starty > 199 - Config::LEDMATRIXHEIGHT) {
      starty = 199 - Config::LEDMATRIXHEIGHT;
    }
  }
  if (slowmove) {
    int16_t diff = startx - oldstartx;
    if (diff > 1) {
      startx = oldstartx + 1;
    } else if (diff < -1) {
      startx = oldstartx - 1;
    }
    oldstartx = startx;
    diff = starty - oldstarty;
    if (diff > 1) {
      starty = oldstarty + 1;
    } else if (diff < -1) {
      starty = oldstarty - 1;
    }
    oldstarty = starty;
  }
  for (uint16_t y = 0; y < 64; y++) {
    for (uint16_t x = 0; x < 64; x++) {
      uint16_t bitmapIdx = (uint16_t)(y + starty) * 320 + (x + startx);
      bitmaptargetcol[y * 64 + x] = c64Colors[bitmap[bitmapIdx] & 0x0f];
    }
  }
}

void LEDMatrixDisplay::drawBitmap(const uint8_t *bitmap,
                                  const uint8_t *vicreg) {
  switch (mode) {
  case 0:
    pixelAreaFollowsSprite(bitmap, vicreg, true);
    break;
  case 1:
    pixelAreaFollowsSprite(bitmap, vicreg, false);
    break;
  case 2:
    mergeXPixels(bitmap);
    break;
  case 3:
    extractEachXthPixel(bitmap);
    break;
  }
  dma_display->drawRGBBitmap(0, 0, bitmaptargetcol, Config::LEDMATRIXWIDTH,
                             Config::LEDMATRIXHEIGHT);
}

void LEDMatrixDisplay::setSpecial1() {
  mode++;
  if (mode > 3) {
    mode = 0;
  }
}

static uint8_t spx[] = "\x13\x10\x30";

void LEDMatrixDisplay::setSpecial2() {
  spritenr++;
  spritenr = spritenr & 7;
  spx[2] = 48 + spritenr;
  ExtCmdQueue::ExternalCmd extcmd;
  extcmd.cmd = ExtCmd::WRITEOSD;
  extcmd.param[2] = (oldstartx + Config::LEDMATRIXWIDTH / 2) / 8;  // x
  extcmd.param[3] = (oldstarty + Config::LEDMATRIXHEIGHT / 2) / 8; // y
  extcmd.param[4] = 3;                                             // w
  extcmd.param[5] = 1;                                             // h
  extcmd.param[6] = 1;                                             // fgcol
  extcmd.param[7] = 0;                                             // bgcol
  extcmd.param[8] = 5;  // duration low
  extcmd.param[9] = 0;  // duration hi
  extcmd.param[10] = 0; // doiidx
  memcpy(&extcmd.param[11], spx, 3);
  ExtCmdQueue::getInstance().push(extcmd);
}

#endif
