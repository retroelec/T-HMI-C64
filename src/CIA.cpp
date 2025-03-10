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

void CIA::checkAlarm() {
  if (isAlarm.load(std::memory_order_acquire)) {
    isAlarm.store(false, std::memory_order_release);
    latchdc0d |= 0x04;
    if (ciareg[0x0d] & 4) {
      latchdc0d |= 0x80;
    }
  }
}

void CIA::checkTimerA(uint8_t deltaT) {
  uint8_t reg0e = ciareg[0x0e];
  if (!(reg0e & 1)) {
    // timer stopped
    return;
  }
  if (reg0e & 0x20) {
    // timer clocked by CNT pin
    return;
  }
  int32_t tmp = timerA - deltaT;
  timerA = (tmp < 0) ? 0 : tmp;
  if (timerA == 0) {
    underflowTimerA = true;
    if (reg0e & 0x02) {
      if (!(reg0e & 0x04)) {
        ciareg[0x01] ^= 0x40;
      }
      // ignore "toggle bit for one cycle"
    }
    latchdc0d |= 0x01;
    if (!(reg0e & 8)) {
      timerA = (latchdc05 << 8) + latchdc04;
    } else {
      ciareg[0x0e] &= 0xfe;
    }
    if (ciareg[0x0d] & 1) {
      latchdc0d |= 0x80;
    }
    if ((ciareg[0x0e] & 0x40) && (serbitnr != 0)) {
      serbitnr--;
      if (serbitnr == 0) {
        latchdc0d |= 0x08;
        if (ciareg[0x0d] & 8) {
          latchdc0d |= 0x80;
        }
        if (serbitnrnext != 0) {
          serbitnr = serbitnrnext;
          serbitnrnext = 0;
        }
      }
    }
  }
}

void CIA::checkTimerB(uint8_t deltaT) {
  uint8_t reg0f = ciareg[0x0f];
  if (!(reg0f & 1)) {
    // timer stopped
    return;
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
    return;
  }
  if (timerB == 0) {
    if (reg0f & 0x02) {
      if (!(reg0f & 0x04)) {
        ciareg[0x01] ^= 0x80;
      }
      // ignore "toggle bit for one cycle"
    }
    latchdc0d |= 0x02;
    if (!(reg0f & 8)) {
      timerB = (latchdc07 << 8) + latchdc06;
    } else {
      ciareg[0x0f] &= 0xfe;
    }
    if (ciareg[0x0d] & 2) {
      latchdc0d |= 0x80;
    }
  }
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

uint8_t CIA::getCommonCIAReg(uint8_t ciaidx) {
  if (ciaidx == 0x04) {
    return timerA & 0xff;
  } else if (ciaidx == 0x05) {
    return (timerA >> 8) & 0xff;
  } else if (ciaidx == 0x06) {
    return timerB & 0xff;
  } else if (ciaidx == 0x07) {
    return (timerB >> 8) & 0xff;
  } else if (ciaidx == 0x08) {
    uint8_t val;
    if (isTODFreezed) {
      val = ciareg[ciaidx];
    } else {
      val = latchrundc08.load(std::memory_order_acquire);
    }
    isTODFreezed = false;
    return val;
  } else if (ciaidx == 0x09) {
    if (isTODFreezed) {
      return ciareg[ciaidx];
    } else {
      return latchrundc09.load(std::memory_order_acquire);
    }
  } else if (ciaidx == 0x0a) {
    if (isTODFreezed) {
      return ciareg[ciaidx];
    } else {
      return latchrundc0a.load(std::memory_order_acquire);
    }
  } else if (ciaidx == 0x0b) {
    isTODFreezed = true;
    ciareg[0x08] = latchrundc08.load(std::memory_order_acquire);
    ciareg[0x09] = latchrundc09.load(std::memory_order_acquire);
    ciareg[0x0a] = latchrundc0a.load(std::memory_order_acquire);
    ciareg[0x0b] = latchrundc0b.load(std::memory_order_acquire);
    return ciareg[ciaidx];
  } else if (ciaidx == 0x0d) {
    uint8_t val = latchdc0d;
    latchdc0d = 0;
    return val;
  } else {
    return ciareg[ciaidx];
  }
}

void CIA::setCommonCIAReg(uint8_t ciaidx, uint8_t val) {
  if (ciaidx == 0x04) {
    latchdc04 = val;
  } else if (ciaidx == 0x05) {
    latchdc05 = val;
    // timerA stopped? if yes, write timerA
    if (!(ciareg[0x0e] & 1)) {
      timerA = (latchdc05 << 8) + latchdc04;
    }
  } else if (ciaidx == 0x06) {
    latchdc06 = val;
  } else if (ciaidx == 0x07) {
    latchdc07 = val;
    // timerB stopped? if yes, write timerB
    if (!(ciareg[0x0f] & 1)) {
      timerB = (latchdc07 << 8) + latchdc06;
    }
  } else if (ciaidx == 0x08) {
    if (ciareg[0x0f] & 128) {
      latchalarmdc08.store(val, std::memory_order_release);
    } else {
      ciareg[0x08] = val;
      latchrundc08.store(val, std::memory_order_release);
      latchrundc09.store(ciareg[0x09], std::memory_order_release);
      latchrundc0a.store(ciareg[0x0a], std::memory_order_release);
      latchrundc0b.store(ciareg[0x0b], std::memory_order_release);
    }
    isTODRunning.store(true, std::memory_order_release);
  } else if (ciaidx == 0x09) {
    if (ciareg[0x0f] & 128) {
      latchalarmdc09.store(val, std::memory_order_release);
    } else {
      ciareg[0x09] = val;
    }
  } else if (ciaidx == 0x0a) {
    if (ciareg[0x0f] & 128) {
      latchalarmdc0a.store(val, std::memory_order_release);
    } else {
      ciareg[0x0a] = val;
    }
  } else if (ciaidx == 0x0b) {
    isTODRunning.store(false, std::memory_order_release);
    if (ciareg[0x0f] & 128) {
      latchalarmdc0b.store(val, std::memory_order_release);
    } else {
      ciareg[0x0b] = val;
    }
  } else if (ciaidx == 0x0c) {
    if (serbitnr == 0) {
      serbitnr = 8;
    } else {
      serbitnrnext = 8;
    }
    ciareg[ciaidx] = val;
  } else if (ciaidx == 0x0d) {
    if (val & 0x80) {
      ciareg[ciaidx] |= val;
    } else {
      ciareg[ciaidx] &= ~(val | 0x80);
    }
  } else if (ciaidx == 0x0e) {
    ciareg[ciaidx] = val;
    if (val & 0x10) {
      timerA = (latchdc05 << 8) + latchdc04;
    }
  } else if (ciaidx == 0x0f) {
    ciareg[ciaidx] = val;
    if (val & 0x10) {
      timerB = (latchdc07 << 8) + latchdc06;
    }
  } else {
    ciareg[ciaidx] = val;
  }
}
