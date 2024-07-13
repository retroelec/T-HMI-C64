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
#ifndef CPUC64_H
#define CPUC64_H

#include "CIA.h"
#include "CPU6502.h"
#include "Joystick.h"
#include "VIC.h"
#include <cstdint>
#include <mutex>

class BLEKB;

class CPUC64 : public CPU6502 {
private:
  uint8_t *ram;
  uint8_t *basicrom;
  uint8_t *kernalrom;
  uint8_t *charrom;
  Joystick joystick;

  uint8_t sidreg[0x100];

  bool bankARAM;
  bool bankDRAM;
  bool bankERAM;
  bool bankDIO;
  uint8_t register1;

  std::mutex pcMutex;

  inline void adaptVICBaseAddrs(bool fromcia) __attribute__((always_inline));
  inline void decodeRegister1(uint8_t val) __attribute__((always_inline));

public:
  VIC *vic;
  BLEKB *blekb;
  CIA cia1;
  CIA cia2;

  CPUC64() : cia1(true), cia2(false) {}

  // public only for logging / debugging
  uint8_t getA();
  uint8_t getX();
  uint8_t getY();
  uint8_t getSP();
  uint8_t getSR();
  uint16_t getPC();

  uint32_t numofcyclespersecond;
  std::atomic<uint16_t> adjustcycles;
  std::atomic<uint16_t> measuredcycles;

  std::atomic<bool> debug;

  // set by class ExternalCmds
  uint8_t joystickmode;
  uint8_t kbjoystickmode;
  bool refreshframecolor;

  bool restorenmi;

  inline uint8_t getMem(uint16_t addr) __attribute__((always_inline));
  inline void setMem(uint16_t addr, uint8_t val) __attribute__((always_inline));

  void cmd6502illegal() override;
  // void cmd6502nop1a() override;
  void run() override;

  void initMemAndRegs();
  void init(uint8_t *ram, uint8_t *charrom, VIC *vic, BLEKB *blekb);
  void setPC(uint16_t pc);
  void exeSubroutine(uint16_t addr, uint8_t rega, uint8_t regx, uint8_t regy);
  void setKeycodes(uint8_t keycode1, uint8_t keycode2);
};

#endif // CPUC64_H
