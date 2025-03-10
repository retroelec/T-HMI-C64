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
#ifndef VIC_H
#define VIC_H

#include "ConfigDisplay.h"
#include <cstdint>

class VIC {
private:
  uint8_t *ram;
  uint16_t *bitmap;
  uint8_t spritespritecoll[320];
  bool spritedatacoll[320];
  uint8_t startbyte;
  ConfigDisplay configDisplay;

  inline void drawByteStdData(uint8_t data, uint16_t &idx, uint16_t &xp,
                              uint16_t col, uint16_t bgcol, uint8_t dx)
      __attribute__((always_inline));
  inline void drawByteMCData(uint8_t data, uint16_t &idx, uint16_t &xp,
                             uint16_t *tftColArr, bool *collArr, uint8_t dx)
      __attribute__((always_inline));
  void drawblankline(uint8_t line);
  inline void shiftDy(uint8_t line, int8_t dy, uint16_t bgcol)
      __attribute__((always_inline));
  inline void shiftDx(uint8_t dx, uint16_t bgcol, uint16_t &idx)
      __attribute__((always_inline));
  inline void drawOnly38ColsFrame(uint16_t tmpidx)
      __attribute__((always_inline));
  inline void drawStdCharModeInt(uint8_t *screenMap, uint16_t bgcol,
                                 uint8_t row, uint8_t dx, uint16_t &xp,
                                 uint16_t idxmap, uint16_t &idx)
      __attribute__((always_inline));
  void drawStdCharMode(uint8_t *screenMap, uint8_t bgColor, uint8_t line,
                       int8_t dy, uint8_t dx);
  inline void drawExtBGColCharModeInt(uint8_t *screenMap, uint8_t *bgColArr,
                                      uint8_t row, uint8_t dx, uint16_t &xp,
                                      uint16_t idxmap, uint16_t &idx)
      __attribute__((always_inline));
  void drawExtBGColCharMode(uint8_t *screenMap, uint8_t *bgColArr, uint8_t line,
                            int8_t dy, uint8_t dx);
  inline void drawMCCharModeInt(uint8_t *screenMap, uint16_t bgcol,
                                uint16_t *tftColArr, uint8_t row, uint8_t dx,
                                uint16_t &xp, uint16_t idxmap, uint16_t &idx)
      __attribute__((always_inline));
  void drawMCCharMode(uint8_t *screenMap, uint8_t bgColor1, uint8_t bgColor2,
                      uint8_t bgColor3, uint8_t line, int8_t dy, uint8_t dx);
  inline void drawMCBitmapModeInt(uint8_t *multicolorBitmap, uint8_t *colorMap1,
                                  uint16_t *tftColArr, uint16_t cidx,
                                  uint16_t mcidx, uint8_t row, uint8_t dx,
                                  uint16_t &xp, uint16_t &idx)
      __attribute__((always_inline));
  void drawMCBitmapMode(uint8_t *multicolorBitmap, uint8_t *colorMap1,
                        uint8_t backgroundColor, uint8_t line, int8_t dy,
                        uint8_t dx);
  inline void drawStdBitmapModeInt(uint8_t *hiresBitmap, uint8_t *colorMap,
                                   uint16_t hiidx, uint16_t &colidx,
                                   uint8_t row, uint8_t dx, uint16_t &xp,
                                   uint16_t &idx)
      __attribute__((always_inline));
  void drawStdBitmapMode(uint8_t *hiresBitmap, uint8_t *colorMap, uint8_t line,
                         int8_t dy, uint8_t dx);
  void drawSpriteDataSC(uint8_t bitnr, int16_t xpos, uint8_t ypos,
                        uint8_t *data, uint8_t color);
  void drawSpriteDataSCDS(uint8_t bitnr, int16_t xpos, uint8_t ypos,
                          uint8_t *data, uint8_t color);
  inline void drawSpriteDataMC2Bits(uint8_t idxc, uint16_t &idx, int16_t &xpos,
                                    uint8_t bitnr, uint16_t *tftcolor)
      __attribute__((always_inline));
  void drawSpriteDataMC(uint8_t bitnr, int16_t xpos, uint8_t ypos,
                        uint8_t *data, uint8_t color10, uint8_t color01,
                        uint8_t color11);
  void drawSpriteDataMCDS(uint8_t bitnr, int16_t xpos, uint8_t ypos,
                          uint8_t *data, uint8_t color10, uint8_t color01,
                          uint8_t color11);
  void drawSprites(uint8_t line);
  inline void checkFrameColor() __attribute__((always_inline));

public:
  // profiling info
  uint8_t cntRefreshs;

  uint8_t *colormap;
  uint8_t *charset;
  uint8_t *chrom;

  uint8_t vicreg[0x40];
  uint8_t latchd011;
  uint8_t latchd012;
  uint16_t vicmem;
  uint16_t bitmapstart;
  uint16_t screenmemstart;
  uint16_t rasterline;
  uint8_t syncd020;
  bool screenblank;

  VIC();
  void initVarsAndRegs();
  void initLCDController();
  void init(uint8_t *ram, uint8_t *charrom);
  void refresh(bool refreshframecolor);
  uint8_t nextRasterline();
  void drawRasterline();
};
#endif // VIC_H
