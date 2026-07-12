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
#ifndef NOTCURSESDISPLAY_H
#define NOTCURSESDISPLAY_H

#include "../Config.h"
#ifdef USE_NOTCURSES_DISPLAY
#include "DisplayDriver.h"
#include <cstdint>
#include <notcurses/notcurses.h>

class NotcursesDisplay : public DisplayDriver {
public:
  static NotcursesDisplay *instance;

  struct notcurses *nc = nullptr;
  struct ncplane *stdplane = nullptr;
  uint8_t currentFrameColor = 6;

  struct notcurses *getNotcursesNC() { return nc; }

private:
  uint32_t c64_to_notcurses_rgb(uint16_t c64_color);

public:
  NotcursesDisplay();
  ~NotcursesDisplay();
  void init() override;
  void drawFrame(uint8_t frameColor) override;
  void drawBitmap(const uint8_t *bitmap, const uint8_t *vicreg) override;
};
#endif

#endif // NOTCURSESDISPLAY_H