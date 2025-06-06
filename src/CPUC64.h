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
#include "ConfigInput.h"
#include "Joystick.h"
#include "SID.h"
#include "VIC.h"
#include <cstdint>
#include <mutex>

class ExternalCmds; // forward declaration

class CPUC64 : public CPU6502 {
private:
  uint8_t *ram;
  uint8_t *basicrom;
  uint8_t *kernalrom;
  uint8_t *charrom;
  Joystick joystick;

  bool bankARAM;
  bool bankDRAM;
  bool bankERAM;
  bool bankDIO;
  uint8_t register1;

  std::mutex pcMutex;

  bool nmiAck;

  inline void vTaskDelayUntilUS(int64_t lastMeasuredTime,
                                uint32_t timeIncrement);
  inline void adaptVICBaseAddrs(bool fromcia) __attribute__((always_inline));
  inline void decodeRegister1(uint8_t val) __attribute__((always_inline));
  inline void checkciatimers(uint8_t cycles) __attribute__((always_inline));
  inline void logDebugInfo() __attribute__((always_inline));
  void check4extcmd();

public:
  VIC *vic;
  CIA cia1;
  CIA cia2;
  SID sid;
  ExternalCmds *externalCmds;
  ConfigInput configInput;

  CPUC64() : cia1(true), cia2(false) {}

  // public only for logging / debugging
  uint8_t getA();
  uint8_t getX();
  uint8_t getY();
  uint8_t getSP();
  uint8_t getSR();
  uint16_t getPC();

  uint32_t numofcyclespersecond;
  uint32_t numofburnedcyclespersecond;

  bool perf = false;
  uint32_t batteryVoltage = 0;
  bool poweroff = false;

  // set by class ExternalCmds
  uint8_t joystickmode;
  uint8_t kbjoystickmode;
  bool deactivateTemp;
  bool debug;
  uint16_t debugstartaddr;
  bool debuggingstarted;
  bool detectreleasekey;

  bool restorenmi;

  uint8_t getMem(uint16_t addr);
  void setMem(uint16_t addr, uint8_t val);

  void cmd6502halt() override;
  void run() override;

  void initMemAndRegs();
  void init(uint8_t *ram, uint8_t *charrom, VIC *vic);
  void setPC(uint16_t pc);
  void exeSubroutine(uint16_t addr, uint8_t rega, uint8_t regx, uint8_t regy);
  void setKeycodes(uint8_t keycode1, uint8_t keycode2);
};

#endif // CPUC64_H
