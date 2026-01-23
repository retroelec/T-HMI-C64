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
#include "VIC.h"
#include "Config.h"
#include "display/DisplayFactory.h"
#include "platform/PlatformManager.h"
#include <cstring>
#include <stdexcept>

static bool collArr[4] = {false, true, true, true};

VIC::VIC() { bitmap = nullptr; }

void VIC::drawemptyline() {
  uint8_t colBM = vicreg[0x20] & 15;
  for (uint16_t i = 0; i < 320; i++) {
    bitmap[idx++] = colBM;
  }
}

bool VIC::shiftDy(uint8_t bgcol) {
  if ((line < deltay) || (deltay < line - 199)) {
    bool only38cols = !(vicreg[0x16] & 8);
    if (only38cols) {
      uint16_t framecol = vicreg[0x20] & 15;
      for (uint8_t xp = 0; xp < 8; xp++) {
        bitmap[idx++] = framecol;
      }
      for (uint16_t xp = 8; xp < 39 * 8; xp++) {
        bitmap[idx++] = bgcol;
      }
      for (uint8_t xp = 0; xp < 8; xp++) {
        bitmap[idx++] = framecol;
      }
    } else {
      for (uint16_t xp = 0; xp < 40 * 8; xp++) {
        bitmap[idx++] = bgcol;
      }
    }
    return true;
  }
  return false;
}

void VIC::shiftDx(uint8_t bgcol) {
  for (uint8_t i = 0; i < deltax; i++) {
    bitmap[idx++] = bgcol;
  }
}

void VIC::drawOnly38ColsFrame(uint16_t tmpidx) {
  bool only38cols = !(vicreg[0x16] & 8);
  if (only38cols) {
    uint8_t framecol = vicreg[0x20] & 15;
    for (uint8_t xp = 0; xp < 8; xp++) {
      bitmap[tmpidx++] = framecol;
    }
  }
}

void VIC::drawByteStdData(uint8_t data, uint16_t &idx, uint16_t &xp,
                          uint8_t col, uint8_t bgcol, uint8_t dx) {
  uint8_t bitval = 128;
  for (uint8_t i = 0; i < 8 - dx; i++) {
    if (data & bitval) {
      bitmap[idx++] = col;
      spritedatacoll[xp++] = true;
    } else {
      bitmap[idx++] = bgcol;
      xp++;
    }
    bitval >>= 1;
  }
}

void VIC::drawByteMCData(uint8_t data, uint16_t &idx, uint16_t &xp,
                         uint8_t *tftColArr, bool *collArr, uint8_t dx) {
  uint8_t bitshift = 6;
  for (uint8_t i = 0; i < (8 - dx) >> 1; i++) {
    uint8_t bitpair = (data >> bitshift) & 0x03;
    uint8_t tftcolor = tftColArr[bitpair];
    bitmap[idx++] = tftcolor;
    bitmap[idx++] = tftcolor;
    spritedatacoll[xp++] = collArr[bitpair];
    spritedatacoll[xp++] = collArr[bitpair];
    bitshift -= 2;
  }
}

void VIC::drawidleline(uint8_t ghostbyte) {
  uint8_t col = 0;
  uint8_t bgcol = vicreg[0x20] & 15;
  uint16_t xp = 0;
  for (uint16_t i = 0; i < 40; i++) {
    drawByteStdData(ghostbyte, idx, xp, col, bgcol, 0);
  }
}

void VIC::drawStdCharModeInt(uint8_t *screenMap, uint8_t bgcol, uint8_t row,
                             uint8_t dx, uint16_t &xp, uint16_t yidx,
                             uint16_t &idx) {
  uint8_t col = colormap[yidx] & 15;
  uint8_t ch = screenMap[yidx];
  uint16_t idxch = ch << 3;
  uint8_t chardata = charset[idxch + row];
  drawByteStdData(chardata, idx, xp, col, bgcol, dx);
}

void VIC::drawStdCharMode(uint8_t *screenMap, uint8_t bgColor) {
  uint8_t bgcol = bgColor & 15;
  if (shiftDy(bgcol)) {
    return;
  }
  shiftDx(bgcol);
  drawStdCharModeInt(screenMap, bgcol, row, 0, xp, yidx++, idx);
  drawOnly38ColsFrame(idx - 8 - deltax);
  for (uint8_t x = 1; x < 39; x++) {
    drawStdCharModeInt(screenMap, bgcol, row, 0, xp, yidx++, idx);
  }
  drawStdCharModeInt(screenMap, bgcol, row, deltax, xp, yidx, idx);
  drawOnly38ColsFrame(idx - 8);
}

void VIC::drawMCCharModeInt(uint8_t *screenMap, uint8_t bgcol,
                            uint8_t *tftColArr, uint8_t row, uint8_t dx,
                            uint16_t &xp, uint16_t yidx, uint16_t &idx) {
  uint8_t colc64 = colormap[yidx] & 15;
  uint8_t ch = screenMap[yidx];
  uint16_t idxch = ch << 3;
  uint8_t chardata = charset[idxch + row];
  if (colc64 & 8) {
    tftColArr[3] = colc64 & 7;
    drawByteMCData(chardata, idx, xp, tftColArr, collArr, dx);
  } else {
    drawByteStdData(chardata, idx, xp, colc64, bgcol, dx);
  }
}

void VIC::drawMCCharMode(uint8_t *screenMap, uint8_t bgColor, uint8_t color1,
                         uint8_t color2) {
  uint8_t bgcol = bgColor & 15;
  if (shiftDy(bgcol)) {
    return;
  }
  shiftDx(bgcol);
  uint8_t tftColArr[4];
  tftColArr[0] = bgcol;
  tftColArr[1] = color1 & 15;
  tftColArr[2] = color2 & 15;
  drawMCCharModeInt(screenMap, bgcol, tftColArr, row, 0, xp, yidx++, idx);
  drawOnly38ColsFrame(idx - 8 - deltax);
  for (uint8_t x = 1; x < 39; x++) {
    drawMCCharModeInt(screenMap, bgcol, tftColArr, row, 0, xp, yidx++, idx);
  }
  drawMCCharModeInt(screenMap, bgcol, tftColArr, row, deltax, xp, yidx, idx);
  drawOnly38ColsFrame(idx - 8);
}

void VIC::drawExtBGColCharModeInt(uint8_t *screenMap, uint8_t *bgColArr,
                                  uint8_t row, uint8_t dx, uint16_t &xp,
                                  uint16_t yidx, uint16_t &idx) {
  uint8_t col = colormap[yidx] & 15;
  uint8_t ch = screenMap[yidx];
  uint8_t ch6bits = ch & 0x3f;
  uint8_t bgcol = bgColArr[ch >> 6] & 15;
  uint16_t idxch = ch6bits << 3;
  uint8_t chardata = charset[idxch + row];
  drawByteStdData(chardata, idx, xp, col, bgcol, dx);
}

void VIC::drawExtBGColCharMode(uint8_t *screenMap, uint8_t *bgColArr) {
  uint8_t bgcol0 = bgColArr[0];
  if (shiftDy(bgcol0)) {
    return;
  }
  shiftDx(bgcol0);
  drawExtBGColCharModeInt(screenMap, bgColArr, row, 0, xp, yidx++, idx);
  drawOnly38ColsFrame(idx - 8 - deltax);
  for (uint8_t x = 1; x < 39; x++) {
    drawExtBGColCharModeInt(screenMap, bgColArr, row, 0, xp, yidx++, idx);
  }
  drawExtBGColCharModeInt(screenMap, bgColArr, row, deltax, xp, yidx, idx);
  drawOnly38ColsFrame(idx - 8);
}

void VIC::drawMCBitmapModeInt(uint8_t *multicolorBitmap, uint8_t *colorMap1,
                              uint8_t *tftColArr, uint16_t cidx, uint16_t mcidx,
                              uint8_t row, uint8_t dx, uint16_t &xp,
                              uint16_t &idx) {
  uint8_t color1 = colorMap1[cidx];
  uint8_t color2 = colormap[cidx];
  tftColArr[1] = (color1 >> 4) & 0x0f;
  tftColArr[2] = color1 & 0x0f;
  tftColArr[3] = color2 & 0x0f;
  uint8_t data = multicolorBitmap[mcidx + row];
  drawByteMCData(data, idx, xp, tftColArr, collArr, dx);
}

void VIC::drawMCBitmapMode(uint8_t *multicolorBitmap, uint8_t *colorMap1,
                           uint8_t backgroundColor) {
  uint8_t tftColArr[4];
  tftColArr[0] = backgroundColor & 0x0f;
  if (shiftDy(tftColArr[0])) {
    return;
  }
  shiftDx(tftColArr[0]);
  uint16_t mcidx = (y * 40) << 3;
  uint16_t xp = 0;
  drawMCBitmapModeInt(multicolorBitmap, colorMap1, tftColArr, yidx++, mcidx,
                      row, 0, xp, idx);
  mcidx += 8;
  drawOnly38ColsFrame(idx - 8 - deltax);
  for (uint8_t x = 1; x < 39; x++) {
    drawMCBitmapModeInt(multicolorBitmap, colorMap1, tftColArr, yidx++, mcidx,
                        row, 0, xp, idx);
    mcidx += 8;
  }
  drawMCBitmapModeInt(multicolorBitmap, colorMap1, tftColArr, yidx, mcidx, row,
                      deltax, xp, idx);
  drawOnly38ColsFrame(idx - 8);
}

void VIC::drawStdBitmapModeInt(uint8_t *hiresBitmap, uint8_t *colorMap,
                               uint16_t hiidx, uint16_t &colidx, uint8_t row,
                               uint8_t dx, uint16_t &xp, uint16_t &idx) {
  uint8_t color = colorMap[colidx++];
  uint8_t colorfg = (color & 0xf0) >> 4;
  uint8_t colorbg = color & 0x0f;
  uint8_t data = hiresBitmap[hiidx + row];
  drawByteStdData(data, idx, xp, colorfg, colorbg, dx);
}

void VIC::drawStdBitmapMode(uint8_t *hiresBitmap, uint8_t *colorMap) {
  // todo: background color is specific for each "tile"
  if (shiftDy(0)) {
    return;
  }
  shiftDx(0);
  uint16_t hiidx = (y * 40) << 3;
  drawStdBitmapModeInt(hiresBitmap, colorMap, hiidx, yidx, row, 0, xp, idx);
  hiidx += 8;
  drawOnly38ColsFrame(idx - 8 - deltax);
  for (uint8_t x = 1; x < 39; x++) {
    drawStdBitmapModeInt(hiresBitmap, colorMap, hiidx, yidx, row, 0, xp, idx);
    hiidx += 8;
  }
  drawStdBitmapModeInt(hiresBitmap, colorMap, hiidx, yidx, row, deltax, xp,
                       idx);
  drawOnly38ColsFrame(idx - 8);
}

void VIC::drawSpriteDataSC(uint8_t bitnr, int16_t xpos, uint8_t ypos,
                           uint8_t *data, uint8_t color) {
  uint16_t idx = xpos + ypos * 320;
  bool only38cols = !(vicreg[0x16] & 8);
  uint16_t low = only38cols ? 8 : 0;
  uint16_t high = only38cols ? 312 : 320;
  for (uint8_t x = 0; x < 3; x++) {
    uint8_t d = *data++;
    uint8_t bitval = 128;
    for (uint8_t i = 0; i < 8; i++) {
      if (xpos < low) {
        idx++;
        xpos++;
        bitval >>= 1;
        continue;
      } else if (xpos >= high) {
        return;
      }
      if (d & bitval) {
        uint8_t bgspriteprio = vicreg[0x1b] & bitnr;
        if (spritedatacoll[xpos]) {
          // sprite - data collision
          vicreg[0x1f] |= bitnr;
        }
        if (bgspriteprio && spritedatacoll[xpos]) {
          // background prio
          idx++;
        } else {
          bitmap[idx++] = color;
        }
        uint8_t sprcoll = spritespritecoll[xpos];
        if (sprcoll != 0) {
          // sprite - sprite collision
          vicreg[0x1e] |= sprcoll | bitnr;
        }
        spritespritecoll[xpos++] = sprcoll | bitnr;
      } else {
        idx++;
        xpos++;
      }
      bitval >>= 1;
    }
  }
}

void VIC::drawSpriteDataSCDS(uint8_t bitnr, int16_t xpos, uint8_t ypos,
                             uint8_t *data, uint8_t color) {
  uint16_t idx = xpos + ypos * 320;
  bool only38cols = !(vicreg[0x16] & 8);
  uint16_t low = only38cols ? 8 : 0;
  uint16_t high = only38cols ? 312 : 320;
  for (uint8_t x = 0; x < 3; x++) {
    uint8_t d = *data++;
    uint8_t bitval = 128;
    for (uint8_t i = 0; i < 8; i++) {
      if (xpos < low) {
        idx += 2;
        xpos += 2;
        bitval >>= 1;
        continue;
      } else if (xpos >= high) {
        return;
      }
      if (d & bitval) {
        uint8_t bgspriteprio = vicreg[0x1b] & bitnr;
        if (spritedatacoll[xpos] || spritedatacoll[xpos + 1]) {
          // sprite - data collision
          vicreg[0x1f] |= bitnr;
        }
        if (bgspriteprio && spritedatacoll[xpos]) {
          // background prio
          idx++;
        } else {
          bitmap[idx++] = color;
        }
        if (bgspriteprio && spritedatacoll[xpos + 1]) {
          // background prio
          idx++;
        } else {
          bitmap[idx++] = color;
        }
        uint8_t sprcoll = spritespritecoll[xpos];
        if (sprcoll != 0) {
          // sprite - sprite collision
          vicreg[0x1e] |= sprcoll | bitnr;
        }
        spritespritecoll[xpos++] = sprcoll | bitnr;
      } else {
        idx += 2;
        xpos += 2;
      }
      bitval >>= 1;
    }
  }
}

void VIC::drawSpriteDataMC2Bits(uint8_t idxc, uint16_t &idx, int16_t &xpos,
                                uint8_t bitnr, uint8_t *tftcolor) {
  bool only38cols = !(vicreg[0x16] & 8);
  uint16_t low = only38cols ? 8 : 0;
  uint16_t high = only38cols ? 312 : 320;
  if (xpos < low) {
    idx += 2;
    xpos += 2;
    return;
  } else if (xpos >= high) {
    return;
  }
  if (idxc) {
    uint8_t bgspriteprio = vicreg[0x1b] & bitnr;
    if (spritedatacoll[xpos] || spritedatacoll[xpos + 1]) {
      // sprite - data collision
      vicreg[0x1f] |= bitnr;
    }
    if (bgspriteprio && spritedatacoll[xpos]) {
      // background prio
      idx++;
    } else {
      bitmap[idx++] = tftcolor[idxc];
    }
    if (bgspriteprio && spritedatacoll[xpos + 1]) {
      // background prio
      idx++;
    } else {
      bitmap[idx++] = tftcolor[idxc];
    }
    uint8_t bitnrcollxpos0 = spritespritecoll[xpos];
    uint8_t bitnrcollxpos1 = spritespritecoll[xpos + 1];
    if (bitnrcollxpos0 != 0) {
      // sprite - sprite collision
      vicreg[0x1e] |= bitnrcollxpos0 | bitnr;
    }
    if (bitnrcollxpos1 != 0) {
      // sprite - sprite collision
      vicreg[0x1e] |= bitnrcollxpos1 | bitnr;
    }
    spritespritecoll[xpos++] = bitnrcollxpos0 | bitnr;
    spritespritecoll[xpos++] = bitnrcollxpos1 | bitnr;
  } else {
    idx += 2;
    xpos += 2;
  }
}

void VIC::drawSpriteDataMC(uint8_t bitnr, int16_t xpos, uint8_t ypos,
                           uint8_t *data, uint8_t color10, uint8_t color01,
                           uint8_t color11) {
  uint8_t tftcolor[4] = {0, color01, color10, color11};
  uint16_t idx = xpos + ypos * 320;
  for (uint8_t x = 0; x < 3; x++) {
    uint8_t d = *data++;
    uint8_t idxc = (d & 192) >> 6;
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
    idxc = (d & 48) >> 4;
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
    idxc = (d & 12) >> 2;
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
    idxc = (d & 3);
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
  }
}

void VIC::drawSpriteDataMCDS(uint8_t bitnr, int16_t xpos, uint8_t ypos,
                             uint8_t *data, uint8_t color10, uint8_t color01,
                             uint8_t color11) {
  uint8_t tftcolor[4] = {0, color01, color10, color11};
  uint16_t idx = xpos + ypos * 320;
  for (uint8_t x = 0; x < 3; x++) {
    uint8_t d = *data++;
    uint8_t idxc = (d & 192) >> 6;
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
    xpos -= 2;
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
    idxc = (d & 48) >> 4;
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
    xpos -= 2;
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
    idxc = (d & 12) >> 2;
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
    xpos -= 2;
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
    idxc = (d & 3);
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
    xpos -= 2;
    drawSpriteDataMC2Bits(idxc, idx, xpos, bitnr, tftcolor);
  }
}

void VIC::drawSprites(uint8_t line) {
  uint8_t spritesenabled = vicreg[0x15];
  uint8_t spritesdoubley = vicreg[0x17];
  uint8_t spritesdoublex = vicreg[0x1d];
  uint8_t multicolorreg = vicreg[0x1c];
  uint8_t color01 = vicreg[0x25] & 0x0f;
  uint8_t color11 = vicreg[0x26] & 0x0f;
  memset(spritespritecoll, 0, sizeof(spritespritecoll));
  uint8_t bitval = 128;
  for (int8_t nr = 7; nr >= 0; nr--) {
    if (spritesenabled & bitval) {
      uint8_t facysize = (spritesdoubley & bitval) ? 2 : 1;
      uint8_t y = vicreg[0x01 + nr * 2];
      if ((line >= y) && (line < (y + 21 * facysize))) {
        int16_t x = vicreg[0x00 + nr * 2] - 24;
        if (vicreg[0x10] & bitval) {
          x += 256;
        }
        uint8_t ypos = line - 50;
        uint16_t dataaddr = ram[screenmemstart + 1016 + nr] * 64;
        uint8_t *data = ram + vicmem + dataaddr + ((line - y) / facysize) * 3;
        uint8_t col = vicreg[0x27 + nr] & 0x0f;
        if (multicolorreg & bitval) {
          if (spritesdoublex & bitval) {
            drawSpriteDataMCDS(bitval, x, ypos, data, col, color01, color11);
          } else {
            drawSpriteDataMC(bitval, x, ypos, data, col, color01, color11);
          }
        } else {
          if (spritesdoublex & bitval) {
            drawSpriteDataSCDS(bitval, x, ypos, data, col);
          } else {
            drawSpriteDataSC(bitval, x, ypos, data, col);
          }
        }
      }
    }
    bitval >>= 1;
  }
  if (vicreg[0x1f] != 0) {
    if (vicreg[0x1a] & 2) {
      vicreg[0x19] |= 0x82;
    } else {
      vicreg[0x19] |= 0x02;
    }
  }
  if (vicreg[0x1e] != 0) {
    if (vicreg[0x1a] & 4) {
      vicreg[0x19] |= 0x84;
    } else {
      vicreg[0x19] |= 0x04;
    }
  }
}

void VIC::initVarsAndRegs() {
  for (uint8_t i = 0; i < 0x40; i++) {
    vicreg[i] = 0;
  }
  vicreg[0x11] = 0x1b;
  vicreg[0x16] = 0xc8;
  vicreg[0x18] = 0x15;
  vicreg[0x19] = 0x71;
  vicreg[0x1a] = 0xf0;

  cntRefreshs.store(0, std::memory_order_release);
  vicmem = 0;
  bitmapstart = 0x2000;
  screenmemstart = 1024;
  rasterline = 0;
  charset = chrom;
  vertborder = true;
  doiactive[0] = false;
  doiactive[1] = false;
}

void VIC::init(uint8_t *ram, const uint8_t *charrom) {
  if (bitmap != nullptr) {
    // init method must be called only once
    return;
  }
  this->ram = ram;
  this->chrom = charrom;

  // allocate bitmap memory to be transfered to LCD
  bitmap = new uint8_t[320 * 200];
  if (bitmap == nullptr) {
    throw std::runtime_error("could not allocate memory for bitmap");
  }

  // init display
  display = Display::create();
  display->init();

  // div init
  colormap = new uint8_t[1024]();
  if (colormap == nullptr) {
    throw std::runtime_error("could not allocate memory for colormap");
  }
  initVarsAndRegs();
}

void VIC::refresh() {
  dispOverlayInfo();
  display->drawBitmap(bitmap);
  display->drawFrame(vicreg[0x20] & 15);
  cntRefreshs.fetch_add(1, std::memory_order_release);
}

void VIC::drawOverlay(uint8_t doiidx) {
  if (!doiactive[doiidx]) {
    return;
  }
  uint8_t cline = line >> 3;
  if ((cline < doistarty[doiidx]) ||
      (cline >= doistarty[doiidx] + doih[doiidx])) {
    return;
  }
  uint8_t crow = doistartx[doiidx];
  uint8_t bgcol = doibgcol[doiidx] & 15;
  uint8_t fgcol = doicol[doiidx] & 15;
  uint16_t idxdoi = line * 320 + crow * 8;
  uint16_t tidxdoi = crow + cline * 40;
  uint8_t rowdoi = line & 7;
  for (uint8_t x = 0; x < doiw[doiidx]; x++) {
    uint8_t ch = doitextmap[tidxdoi++];
    uint16_t idxch = ch << 3;
    uint8_t data = chrom[idxch + rowdoi];
    uint8_t bitval = 128;
    for (uint8_t i = 0; i < 8; i++) {
      if (data & bitval) {
        bitmap[idxdoi++] = fgcol;
      } else {
        bitmap[idxdoi++] = bgcol;
      }
      bitval >>= 1;
    }
  }
}

uint8_t VIC::nextRasterline() {
  rasterline++;
  uint8_t d011 = vicreg[0x11];
  if (rasterline > 311) {
    rasterline = 0;
    lineC64map = 0;
  } else if (rasterline == 49) {
    denbadline = (d011 & 0x10) ? true : false;
  }
  bool rsel = d011 & 8;
  if ((rasterline == 51) && rsel && denbadline) {
    vertborder = false;
  } else if ((rasterline == 55) && (!rsel) && denbadline) {
    vertborder = false;
  } else if ((rasterline == 247) && (!rsel)) {
    vertborder = true;
  } else if ((rasterline == 251) && rsel) {
    vertborder = true;
  }
  uint8_t raster8 = (rasterline >= 256) ? 0x80 : 0;
  uint8_t raster7 = (rasterline & 0xff);
  vicreg[0x12] = raster7;
  if ((latchd012 == raster7) && ((latchd011 & 0x80) == raster8)) {
    if (vicreg[0x1a] & 1) {
      vicreg[0x19] |= 0x81;
    } else {
      vicreg[0x19] |= 0x01;
    }
  }
  // calculate cycles used by VIC
  if (!denbadline) {
    return 0;
  }
  uint8_t viccycles = 0;
  // badline?
  if (((vicreg[0x11] & 7) == (rasterline & 7)) && (rasterline >= 0x30) &&
      (rasterline <= 0xf7)) {
    viccycles = 40;
    lineC64map++;
    caccbadlinecnt = 8;
  }
  // active sprites?
  uint8_t spritesenabled = vicreg[0x15];
  uint8_t spritesdoubley = vicreg[0x17];
  uint8_t mask = 0x80;
  for (int nr = 7; nr >= 0; --nr, mask >>= 1) {
    if (spritesenabled & mask) {
      uint8_t height = (spritesdoubley & mask) ? 42 : 21;
      uint8_t y = vicreg[0x01 + nr * 2];
      if ((uint8_t)(rasterline - y) < height) {
        viccycles += 2;
      }
    }
  }
  return viccycles;
}

void VIC::drawRasterline() {
  if ((rasterline >= 51) && (rasterline < 251)) {
    line = rasterline - 51;
    idx = line * 320;
    if (!vertborder) {
      uint8_t d011 = vicreg[0x11];
      deltay = (d011 & 7) - 3;
      bool ecm = d011 & 64;
      if ((caccbadlinecnt > 0) || (deltay < line - 199)) {
        caccbadlinecnt--;
        memset(spritedatacoll, false, sizeof(bool) * sizeof(spritedatacoll));
        uint8_t d016 = vicreg[0x16];
        deltax = d016 & 7;
        bool bmm = d011 & 32;
        bool mcm = d016 & 16;
        // variables for the draw* methods
        y = lineC64map - 1;
        row = ((line & 7) - deltay) & 7;
        yidx = y * 40;
        xp = 0;
        if (bmm) {
          if (mcm) {
            drawMCBitmapMode(ram + bitmapstart, ram + screenmemstart,
                             vicreg[0x21]);
          } else {
            drawStdBitmapMode(ram + bitmapstart, ram + screenmemstart);
          }
        } else {
          if ((!ecm) && (!mcm)) {
            drawStdCharMode(ram + screenmemstart, vicreg[0x21]);
          } else if ((!ecm) && mcm) {
            drawMCCharMode(ram + screenmemstart, vicreg[0x21], vicreg[0x22],
                           vicreg[0x23]);
          } else if (ecm && (!mcm)) {
            uint8_t bgColArr[] = {vicreg[0x21], vicreg[0x22], vicreg[0x23],
                                  vicreg[0x24]};
            drawExtBGColCharMode(ram + screenmemstart, bgColArr);
          }
        }
      } else {
        uint8_t ghostbyte = ecm ? ram[vicmem + 0x39ff] : ram[vicmem + 0x3fff];
        drawidleline(ghostbyte);
      }
      drawSprites(rasterline - 1);
      // draw overlay
      drawOverlay(0);
      drawOverlay(1);
    } else {
      drawemptyline();
    }
  }
}

void VIC::drawDOIBox(uint8_t *box, uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                     uint8_t fgcol, uint8_t bgcol, uint16_t duration,
                     uint8_t doiidx) {
  doistartx[doiidx] = x;
  doistarty[doiidx] = y;
  doiw[doiidx] = w;
  doih[doiidx] = h;
  doicol[doiidx] = fgcol;
  doibgcol[doiidx] = bgcol;
  doiduration[doiidx] = ((int64_t)duration) * 1000 * 1000;
  for (uint8_t yi = 0; yi < h; yi++) {
    const uint8_t *sourceLine = &box[yi * w];
    uint8_t *destLine = &doitextmap[(y + yi) * 40 + x];
    std::copy(sourceLine, sourceLine + w, destLine);
  }
  doistarttime[doiidx] = PlatformManager::getInstance().getTimeUS();
  doiactive[doiidx] = true;
}

void VIC::dispOverlayInfoInt(uint8_t doiidx) {
  if (!doiactive[doiidx]) {
    return;
  }
  int64_t acttime = PlatformManager::getInstance().getTimeUS();
  if (acttime - doistarttime[doiidx] >= doiduration[doiidx]) {
    doiactive[doiidx] = false;
    return;
  }
}

void VIC::dispOverlayInfo() {
  dispOverlayInfoInt(1);
  dispOverlayInfoInt(0);
}
