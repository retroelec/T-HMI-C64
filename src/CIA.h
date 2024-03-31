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
#ifndef CIA_H
#define CIA_H

#include <atomic>
#include <cstdint>

// register dc0d:
// - Interrupt Control Register when written to
// - is an Interrupt Latch Register when read from
// - read the clear-on-read register $dc0d will ACK all pending CIA 1 interrupts

class CIA {
private:
  std::atomic<bool> triggerTimerB;

public:
  std::atomic<uint8_t> ciareg[0x10];

  std::atomic<uint8_t> latchdc04;
  std::atomic<uint8_t> latchdc05;
  std::atomic<uint8_t> latchdc06;
  std::atomic<uint8_t> latchdc07;
  std::atomic<uint8_t> latchdc0d; // read latch register
  std::atomic<uint16_t> timerA;
  std::atomic<uint16_t> timerB;
  std::atomic<bool> reloadA;
  std::atomic<bool> reloadB;

  bool checkTimerA(uint8_t deltaT);
  bool checkTimerB(uint8_t deltaT);
  void init();
};
#endif // CIA_H
