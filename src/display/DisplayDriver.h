/*
 Copyright (C) 2024-2025 retroelec <retroelec42@gmail.com>

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
   * @param frameColor 16-bit color value.
   */
  virtual void drawFrame(uint16_t frameColor) = 0;

  /**
   * @brief Draws the provided bitmap.
   *
   * The bitmap contains pixel data in 16-bit RGB565 format.
   *
   * @param bitmap Pointer to the bitmap data.
   */
  virtual void drawBitmap(uint16_t *bitmap) = 0;

  /**
   * @brief Provides access to the C64 palette in display-native format.
   *
   * Returns a pointer to an array of 16 color entries corresponding to
   * the Commodore 64 color palette.
   *
   * @return Pointer to a constant array of 16 bit colors.
   */
  virtual const uint16_t *getC64Colors() const = 0;

  virtual ~DisplayDriver() {}
};

#endif // DISPLAYDRIVER_H
