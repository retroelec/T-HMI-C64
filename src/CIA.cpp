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

bool CIA::checkTimerA(uint8_t deltaT) {
  if ((ciareg[0x0e] & 1) == 0) {
    // timer stopped
    return false;
  }
  if (ciareg[0x0e] & 0x20) {
    // timer clocked by CNT pin
    return false;
  }
  int32_t tmp = timerA.load(std::memory_order_acquire) - deltaT;
  timerA.store((tmp < 0) ? 0 : tmp, std::memory_order_release);
  if (timerA.load(std::memory_order_acquire) == 0) {
    triggerTimerB.store(true, std::memory_order_release);
    latchdc0d.fetch_or(0x01, std::memory_order_release);
    if (reloadA.load(std::memory_order_acquire)) {
      timerA.store((latchdc05.load(std::memory_order_acquire) << 8) +
                       latchdc04.load(std::memory_order_acquire),
                   std::memory_order_release);
    }
    if (ciareg[0x0d] & 1) {
      latchdc0d.fetch_or(0x80, std::memory_order_release);
      return true;
    }
  }
  return false;
}

bool CIA::checkTimerB(uint8_t deltaT) {
  if ((ciareg[0x0f] & 1) == 0) {
    // timer stopped
    return false;
  }
  if (((ciareg[0x0f] & 0x60) == 0) ||
      (triggerTimerB && ((ciareg[0x0f] & 0x60) == 0x40))) {
    int32_t tmp = timerB.load(std::memory_order_acquire) - deltaT;
    timerB.store((tmp < 0) ? 0 : tmp, std::memory_order_release);
  } else {
    return false;
  }
  if (timerB.load(std::memory_order_acquire) == 0) {
    latchdc0d.fetch_or(0x02, std::memory_order_release);
    if (reloadB.load(std::memory_order_acquire)) {
      timerB.store((latchdc07.load(std::memory_order_acquire) << 8) +
                       latchdc06.load(std::memory_order_acquire),
                   std::memory_order_release);
    }
    if (ciareg[0x0d] & 2) {
      latchdc0d.fetch_or(0x80, std::memory_order_release);
      return true;
    }
  }
  return false;
}

void CIA::init() {
  triggerTimerB.store(false, std::memory_order_release);
  latchdc04.store(0x1a, std::memory_order_release);
  latchdc05.store(0x41, std::memory_order_release);
  timerA.store(0x411a, std::memory_order_release);
  latchdc06.store(0, std::memory_order_release);
  latchdc07.store(0, std::memory_order_release);
  timerB.store(0, std::memory_order_release);
  latchdc0d.store(0x01, std::memory_order_release);
  ciareg[0x0e].store(0x01, std::memory_order_release);
  ciareg[0x0f].store(0, std::memory_order_release);
}
