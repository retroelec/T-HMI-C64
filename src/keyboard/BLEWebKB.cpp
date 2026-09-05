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
#ifdef USE_BLEWEB_KEYBOARD

#include "BLEWebKB.h"

void BLEWebKB::init() {
  dc01dc00.store(0xffff, std::memory_order_release);
  shiftctrlcode.store(0, std::memory_order_release);
  joyvalue.store(0xff, std::memory_order_release);
  blekb.init();
  webkb.init();
}

void BLEWebKB::scanKeyboard() {
  blekb.scanKeyboard();
  webkb.scanKeyboard();

  uint8_t dc01 = blekb.getKBCodeDC01() & webkb.getKBCodeDC01();
  uint8_t dc00 = blekb.getKBCodeDC00() & webkb.getKBCodeDC00();
  dc01dc00.store((static_cast<uint16_t>(dc01) << 8) | dc00,
                 std::memory_order_release);

  uint8_t sc = blekb.getShiftctrlcode() | webkb.getShiftctrlcode();
  shiftctrlcode.store(sc, std::memory_order_release);

  uint8_t joy = blekb.getKBJoyValue();
  joyvalue.store(joy, std::memory_order_release);
}

uint8_t BLEWebKB::getKBCodeDC01() {
  return static_cast<uint8_t>(dc01dc00.load(std::memory_order_acquire) >> 8);
}

uint8_t BLEWebKB::getKBCodeDC00() {
  return static_cast<uint8_t>(dc01dc00.load(std::memory_order_acquire) & 0xFF);
}

uint8_t BLEWebKB::getShiftctrlcode() {
  return shiftctrlcode.load(std::memory_order_acquire);
}

uint8_t BLEWebKB::getKBJoyValue() {
  return joyvalue.load(std::memory_order_acquire);
}

void BLEWebKB::sendExtCmdNotification(uint8_t *data, size_t size) {
  blekb.sendExtCmdNotification(data, size);
}

void BLEWebKB::setDetectReleasekey(bool detectreleasekey) {
  blekb.setDetectReleasekey(detectreleasekey);
}
#endif
