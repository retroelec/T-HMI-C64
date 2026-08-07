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
#ifndef TERMINALTEXTDISPLAY_H
#define TERMINALTEXTDISPLAY_H

#include "../Config.h"
#ifdef USE_TERMINALTEXT_DISPLAY

#include "DisplayDriver.h"
#include <cstdint>
#include <termios.h>

class TerminalTextDisplay : public DisplayDriver {
private:
  uint8_t frameColor;
  uint16_t prevScreenMemStart;
  uint8_t prevBgColor;
  uint8_t prevScreen[1000];

  void setRawMode();
  void restoreTerminal();
  static uint32_t petsciiToUnicode(uint8_t c);
  static void setAnsiBgColor(uint8_t c64color);
  void renderScreen(const uint8_t *ram, const uint8_t *vicreg);

public:
  TerminalTextDisplay();
  ~TerminalTextDisplay() override;
  void init() override;
  void drawFrame(uint8_t frameColor) override;
  void drawBitmap(const uint8_t *bitmap) override {}
  void drawBitmap(const uint8_t *bitmap, const uint8_t *vicreg,
                  const uint8_t *ram) override;
};

#endif
#endif // TERMINALTEXTDISPLAY_H