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

bool CIA::checkAlarm() {
  if (isAlarm.load(std::memory_order_acquire)) {
    isAlarm.store(false, std::memory_order_release);
    latchdc0d |= 0x04;
    if (ciareg[0x0d] & 4) {
      latchdc0d |= 0x80;
      return true;
    }
  }
  return false;
}

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
  bool ret = false;
  int32_t tmp = timerA - deltaT;
  timerA = (tmp < 0) ? 0 : tmp;
  if (timerA == 0) {
    underflowTimerA = true;
    if (reg0e & 0x02) {
      if (!(reg0e & 0x04)) {
        ciareg[0x01] ^= 0x40;
      }
      // ignore toggle bit for one cycle ("hardware feature")
    }
    latchdc0d |= 0x01;
    if (!(reg0e & 8)) {
      timerA = (latchdc05 << 8) + latchdc04;
    }
    if (ciareg[0x0d] & 1) {
      latchdc0d |= 0x80;
      ret = true;
    }
    if ((ciareg[0x0e] & 0x40) && (serbitnr != 0)) {
      serbitnr--;
      if (serbitnr == 0) {
        latchdc0d |= 0x08;
        if (ciareg[0x0d] & 8) {
          latchdc0d |= 0x80;
          ret = true;
        }
        if (serbitnrnext != 0) {
          serbitnr = serbitnrnext;
          serbitnrnext = 0;
        }
      }
    }
  }
  return ret;
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
    if (underflowTimerA) {
      underflowTimerA = false;
      timerB--;
    }
  } else {
    return false;
  }
  if (timerB == 0) {
    if (reg0f & 0x02) {
      if (!(reg0f & 0x04)) {
        ciareg[0x01] ^= 0x80;
      }
      // ignore toggle bit for one cycle ("hardware feature")
    }
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

void CIA::init(bool isCIA1) {
  for (uint8_t i = 0; i < 0x10; i++) {
    ciareg[i] = 0;
  }

  underflowTimerA = false;
  serbitnr = 0;
  serbitnrnext = 0;
  latchdc04 = 0;
  latchdc05 = 0;
  latchdc06 = 0;
  latchdc07 = 0;
  latchdc0d = 0;
  timerA = 0;
  timerB = 0;

  isTODRunning.store(false, std::memory_order_release);
  isTODFreezed = false;
  isAlarm.store(false, std::memory_order_release);
  latchrundc08.store(0, std::memory_order_release);
  latchrundc09.store(0, std::memory_order_release);
  latchrundc0a.store(0, std::memory_order_release);
  latchrundc0b.store(0, std::memory_order_release);
  latchalarmdc08.store(0, std::memory_order_release);
  latchalarmdc09.store(0, std::memory_order_release);
  latchalarmdc0a.store(0, std::memory_order_release);
  latchalarmdc0b.store(0, std::memory_order_release);

  if (isCIA1) {
    ciareg[0] = 127;
    ciareg[1] = 255;
    ciareg[2] = 255;
  } else {
    ciareg[0] = 151;
    ciareg[1] = 255;
    ciareg[2] = 63;
  }
}

CIA::CIA(bool isCIA1) { init(isCIA1); }
