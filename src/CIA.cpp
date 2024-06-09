/*
 Copyright (C) 2024 retroelec <retroelec42@gmail.com>

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
#include "CIA.h"

// bit 4 of ciareg[0x0e] and ciareg[0x0f] is handled in CPUC64::setMem

bool CIA::checkTimerA(uint8_t deltaT) {
  uint8_t reg0e = ciareg[0x0e];
  if (!(reg0e & 1)) {
    // timer stopped
    return false;
  }
  if (reg0e & 0x20) {
    // timer clocked by CNT pin
    return false;
  }
  int32_t tmp = timerA - deltaT;
  timerA = (tmp < 0) ? 0 : tmp;
  if (timerA == 0) {
    if (reg0e & 0x02) {
      if (reg0e & 0x04) {
        ciareg[0x0f] |= 0x40;
      } else {
        ciareg[0x0f] ^= 0x40;
      }
    }
    latchdc0d |= 0x01;
    if (!(reg0e & 8)) {
      timerA = (latchdc05 << 8) + latchdc04;
    }
    if (ciareg[0x0d] & 1) {
      latchdc0d |= 0x80;
      return true;
    }
  }
  return false;
}

bool CIA::checkTimerB(uint8_t deltaT) {
  uint8_t reg0f = ciareg[0x0f];
  if (!(reg0f & 1)) {
    // timer stopped
    return false;
  }
  uint8_t bit56 = ciareg[0x0f] & 0x60;
  if (bit56 == 0) {
    int32_t tmp = timerB - deltaT;
    timerB = (tmp < 0) ? 0 : tmp;
  } else if (bit56 == 0x40) {
    timerB--;
  } else {
    return false;
  }
  if (timerB == 0) {
    latchdc0d |= 0x02;
    if (!(reg0f & 8)) {
      timerB = (latchdc07 << 8) + latchdc06;
    }
    if (ciareg[0x0d] & 2) {
      latchdc0d |= 0x80;
      return true;
    }
  }
  return false;
}

void CIA::init() {
  for (uint8_t i = 0; i < 0x10; i++) {
    ciareg[i] = 0;
  }

  uint8_t latchdc04 = 0;
  uint8_t latchdc05 = 0;
  uint8_t latchdc06 = 0;
  uint8_t latchdc07 = 0;
  uint8_t latchdc0d = 0;
  uint16_t timerA = 0;
  uint16_t timerB = 0;
}

CIA::CIA() { init(); }
