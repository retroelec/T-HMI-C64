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
#ifndef BLEWEBKB_H
#define BLEWEBKB_H

#include "../Config.h"
#ifdef USE_BLEWEB_KEYBOARD

#include "BLEKB.h"
#include "WebKB.h"
#include <atomic>

class BLEWebKB : public KeyboardDriver {
public:
  void init() override;
  void scanKeyboard() override;
  uint8_t getKBCodeDC01() override;
  uint8_t getKBCodeDC00() override;
  uint8_t getShiftctrlcode() override;
  uint8_t getKBJoyValue() override;
  void sendExtCmdNotification(uint8_t *data, size_t size) override;
  void setDetectReleasekey(bool detectreleasekey) override;

private:
  BLEKB blekb;
  WebKB webkb;
  std::atomic<uint16_t> dc01dc00;
  std::atomic<uint8_t> shiftctrlcode;
  std::atomic<uint8_t> joyvalue;
};

#endif
#endif // BLEWEBKB_H
