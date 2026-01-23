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
#ifndef BITMAP_UTILS_H
#define BITMAP_UTILS_H

#include <cstdint>

class BitmapUtils {
public:
  static inline void getBitmap(const uint8_t *__restrict__ src,
                               uint16_t *__restrict__ dst,
                               const uint16_t *__restrict__ colors,
                               uint16_t count) {
    // loop-unrolling
    while (count >= 4) {
      dst[0] = colors[src[0] & 0x0f];
      dst[1] = colors[src[1] & 0x0f];
      dst[2] = colors[src[2] & 0x0f];
      dst[3] = colors[src[3] & 0x0f];
      dst += 4;
      src += 4;
      count -= 4;
    }
    while (count--) {
      *dst++ = colors[*src++ & 0x0f];
    }
  }
};

#endif
