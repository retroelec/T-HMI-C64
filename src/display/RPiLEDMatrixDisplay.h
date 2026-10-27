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
#ifndef RPILEDMATRIXDISPLAY_H
#define RPILEDMATRIXDISPLAY_H

#include "../Config.h"
#ifdef USE_RPILED_DISPLAY
#include "DisplayDriver.h"
#include "led-matrix.h"
#include <cstdint>

extern int savedArgc;
extern char **savedArgv;

class RPiLEDMatrixDisplay : public DisplayDriver {
private:
  rgb_matrix::RGBMatrix *matrix;
  rgb_matrix::FrameCanvas *offscreen;
  uint8_t frameColor;
  uint8_t mode = 0;

  void convertPixel(uint8_t c64color, uint8_t &r, uint8_t &g, uint8_t &b);
  void fillBackground(uint8_t r, uint8_t g, uint8_t b);
  void copyViewport(const uint8_t *bitmap);

public:
  RPiLEDMatrixDisplay();
  ~RPiLEDMatrixDisplay() override;

  void init() override;
  void drawFrame(uint8_t frameColor) override;
  void drawBitmap(const uint8_t *bitmap) override;
  void setSpecial1() override;
};
#endif
#endif