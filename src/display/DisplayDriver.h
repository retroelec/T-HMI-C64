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
#ifndef DISPLAYDRIVER_H
#define DISPLAYDRIVER_H

#include <cstdint>

/**
 * @brief Interface for display drivers.
 *
 * This interface defines the required functionality for rendering
 * video output on a specific display. Concrete implementations of this
 * interface handle the hardware-specific details of initializing the
 * display, drawing frames and rendering bitmaps.
 */
class DisplayDriver {
protected:
  static const uint16_t c64_black = 0x0000;
  static const uint16_t c64_white = 0xffff;
  static const uint16_t c64_red = 0x8000;
  static const uint16_t c64_turquoise = 0xa7fc;
  static const uint16_t c64_purple = 0xc218;
  static const uint16_t c64_green = 0x064a;
  static const uint16_t c64_blue = 0x0014;
  static const uint16_t c64_yellow = 0xe74e;
  static const uint16_t c64_orange = 0xd42a;
  static const uint16_t c64_brown = 0x6200;
  static const uint16_t c64_lightred = 0xfbae;
  static const uint16_t c64_grey1 = 0x3186;
  static const uint16_t c64_grey2 = 0x73ae;
  static const uint16_t c64_lightgreen = 0xa7ec;
  static const uint16_t c64_lightblue = 0x043f;
  static const uint16_t c64_grey3 = 0xb5d6;

  static constexpr uint16_t c64Colors[16] = {
      c64_black,  c64_white,      c64_red,       c64_turquoise,
      c64_purple, c64_green,      c64_blue,      c64_yellow,
      c64_orange, c64_brown,      c64_lightred,  c64_grey1,
      c64_grey2,  c64_lightgreen, c64_lightblue, c64_grey3};

public:
  /**
   * @brief Initializes the display hardware.
   *
   * This method is called once during system startup to configure
   * and prepare the display for rendering.
   */
  virtual void init() = 0;

  /**
   * @brief Draws the frame with a uniform color.
   *
   * @param frameColor 8-bit C64 color value.
   */
  virtual void drawFrame(uint8_t frameColor) = 0;

  /**
   * @brief Draws the provided bitmap.
   *
   * The bitmap contains pixel data in 8-bit C64 color data format.
   *
   * @param bitmap Pointer to the bitmap data.
   */
  virtual void drawBitmap(uint8_t *bitmap) = 0;

  virtual ~DisplayDriver() {}

#if defined(BOARD_CYD)
  virtual void reconfigureSPI() = 0;
#endif
};

#endif // DISPLAYDRIVER_H
