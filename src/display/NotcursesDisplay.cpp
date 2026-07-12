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
#ifdef USE_NOTCURSES_DISPLAY
#include "BitmapUtils.h"
#include "NotcursesDisplay.h"
#include <notcurses/notcurses.h>
#include <stdexcept>

NotcursesDisplay *NotcursesDisplay::instance = nullptr;

NotcursesDisplay::NotcursesDisplay() { instance = this; }

NotcursesDisplay::~NotcursesDisplay() {
  if (instance == this) {
    instance = nullptr;
  }
  if (nc) {
    notcurses_stop(nc);
  }
}

uint32_t NotcursesDisplay::c64_to_notcurses_rgb(uint16_t c64_color) {
  uint8_t r = ((c64_color >> 11) & 0x1F) * 255 / 31;
  uint8_t g = ((c64_color >> 5) & 0x3F) * 255 / 63;
  uint8_t b = (c64_color & 0x1F) * 255 / 31;
  return (r << 16) | (g << 8) | b;
}

void NotcursesDisplay::init() {
  notcurses_options ncopts = {};
  ncopts.flags = NCOPTION_NO_ALTERNATE_SCREEN | NCOPTION_SUPPRESS_BANNERS;
  nc = notcurses_init(&ncopts, stderr);
  if (!nc) {
    throw std::runtime_error("notcurses_init failed");
  }
  stdplane = notcurses_stdplane(nc);
  if (!stdplane) {
    throw std::runtime_error("notcurses_stdplane failed");
  }
}

void NotcursesDisplay::drawFrame(uint8_t frameColor) {
  currentFrameColor = frameColor;
}

void NotcursesDisplay::drawBitmap(const uint8_t *bitmap,
                                  const uint8_t *vicreg) {
  (void)vicreg;
  unsigned int term_rows, term_cols;
  ncplane_dim_yx(stdplane, &term_rows, &term_cols);

  const int C64_WIDTH = 320;
  const int C64_HEIGHT = 200;

  for (unsigned int y = 0; y < term_rows && y < C64_HEIGHT; ++y) {
    for (unsigned int x = 0; x < term_cols && x < C64_WIDTH; ++x) {
      uint8_t color_idx = bitmap[y * C64_WIDTH + x];
      uint16_t color_rgb565 = c64Colors[color_idx];
      uint32_t rgb = c64_to_notcurses_rgb(color_rgb565);

      nccell c = {};
      nccell_set_bg_rgb(&c, rgb);
      nccell_load_char(stdplane, &c, ' ');
      ncplane_putc_yx(stdplane, y, x, &c);
      nccell_release(stdplane, &c);
    }
  }
  notcurses_render(nc);
}
#endif