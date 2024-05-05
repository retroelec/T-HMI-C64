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
  int32_t tmp = timerA - deltaT;
  timerA = (tmp < 0) ? 0 : tmp;
  if (timerA == 0) {
    triggerTimerB = true;
    latchdc0d |= 0x01;
    if (reloadA) {
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
  if ((ciareg[0x0f] & 1) == 0) {
    // timer stopped
    return false;
  }
  if (((ciareg[0x0f] & 0x60) == 0) ||
      (triggerTimerB && ((ciareg[0x0f] & 0x60) == 0x40))) {
    int32_t tmp = timerB - deltaT;
    timerB = (tmp < 0) ? 0 : tmp;
  } else {
    return false;
  }
  if (timerB == 0) {
    latchdc0d |= 0x02;
    if (reloadB) {
      timerB = (latchdc07 << 8) + latchdc06;
    }
    if (ciareg[0x0d] & 2) {
      latchdc0d |= 0x80;
      return true;
    }
  }
  return false;
}

CIA::CIA() { triggerTimerB = false; }
