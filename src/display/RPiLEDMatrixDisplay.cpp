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
#ifdef USE_RPILED_DISPLAY
#include "../platform/PlatformManager.h"
#include "RPiLEDMatrixDisplay.h"
#include <cstdint>

static const char *TAG = "RPiLEDMatrix";

RPiLEDMatrixDisplay::RPiLEDMatrixDisplay()
    : matrix(nullptr), offscreen(nullptr), frameColor(0) {}

RPiLEDMatrixDisplay::~RPiLEDMatrixDisplay() { delete matrix; }

void RPiLEDMatrixDisplay::init() {
  rgb_matrix::RGBMatrix::Options opts;
  opts.rows = Config::RPI_PANEL_HEIGHT;
  opts.cols = Config::RPI_PANEL_WIDTH;
  opts.chain_length = Config::RPI_CHAIN_LENGTH;
  opts.parallel = Config::RPI_PARALLEL;
  opts.hardware_mapping = "regular";
  opts.show_refresh_rate = false;
  opts.brightness = Config::RPI_BRIGHTNESS;

  rgb_matrix::RuntimeOptions rt;
  rt.gpio_slowdown = Config::RPI_GPIO_SLOWDOWN;
  rt.drop_privileges = 1;

  int argc = savedArgc;
  char **argv = savedArgv;
  matrix = rgb_matrix::RGBMatrix::CreateFromFlags(&argc, &argv, &opts, &rt);
  if (!matrix) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                       "failed to create RGBMatrix");
    return;
  }
  offscreen = matrix->CreateFrameCanvas();
  if (!offscreen) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                       "failed to create FrameCanvas");
    return;
  }
  PlatformManager::getInstance().log(LOG_INFO, TAG,
                                     "RPi LED Matrix initialized");
}

void RPiLEDMatrixDisplay::drawFrame(uint8_t frameColor) {
  this->frameColor = frameColor;
}

void RPiLEDMatrixDisplay::convertPixel(uint8_t c64color, uint8_t &r, uint8_t &g,
                                       uint8_t &b) {
  uint16_t rgb565 = c64Colors[c64color & 0x0f];
  r = ((rgb565 >> 11) & 0x1f) << 3;
  g = ((rgb565 >> 5) & 0x3f) << 2;
  b = (rgb565 & 0x1f) << 3;
}

void RPiLEDMatrixDisplay::fillBackground(uint8_t r, uint8_t g, uint8_t b) {
  offscreen->Fill(r, g, b);
}

void RPiLEDMatrixDisplay::copyViewport(const uint8_t *bitmap) {
  const int16_t canvasWidth =
      Config::RPI_PANEL_WIDTH * Config::RPI_CHAIN_LENGTH;
  const int16_t canvasHeight = Config::RPI_PANEL_HEIGHT * Config::RPI_PARALLEL;

  for (int16_t y = 0; y < canvasHeight; y++) {
    for (int16_t x = 0; x < canvasWidth; x++) {
      uint8_t c64color = bitmap[y * 320 + x];
      uint8_t r, g, b;
      convertPixel(c64color, r, g, b);
      offscreen->SetPixel(x, y, r, g, b);
    }
  }
}

void RPiLEDMatrixDisplay::drawBitmap(const uint8_t *bitmap) {
  if (!matrix || !offscreen) {
    return;
  }

  copyViewport(bitmap);

  offscreen = matrix->SwapOnVSync(offscreen);
}
#endif
