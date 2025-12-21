/*
 Copyright (C) 2024-2025 retroelec <retroelec42@gmail.com>

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
#ifndef C64SYS_H
#define C64SYS_H

#include "CIA.h"
#include "CPU6502.h"
#include "Floppy.h"
#include "Hooks.h"
#include "IDebugBus.h"
#include "SID.h"
#include "VIC.h"
#include "joystick/JoystickDriver.h"
#include "keyboard/C64Keycodes.h"
#include "keyboard/KeyboardDriver.h"
#include <atomic>
#include <cstdint>

class ExternalCmds; // forward declaration

enum class SpecialJoyModeState { NONE, CHOOSEFILE, RUN, INGAME };

class C64Sys : public CPU6502, public IDebugBus {
private:
  uint8_t *ram;
  uint8_t *kernalrom;
  const uint8_t *charrom;
  JoystickDriver *joystick;

  bool bankARAM;
  bool bankDRAM;
  bool bankERAM;
  bool bankDIO;
  uint8_t register1;

  bool nmiAck;

  SpecialJoyModeState specialjoymodestate;
  uint16_t specialjoymodecnt;
  bool specialjoymode;
  bool gmprevfire1;
  bool gmprevup;
  bool gmprevdown;
  bool gmprevleft;
  bool gmprevright;
  uint8_t savexpos;
  uint8_t saveypos;
  bool liststartflag;
  std::string actfilename;
  struct TextKeycode {
    CodeTriple text;
    CodeTriple keycode;
  };
  CodeTriple actInGameKeycode;
  std::vector<TextKeycode> listInGameKeycodes = {
      {{39, 32, 39}, C64_KEYCODE_SPACE}, {{39, 49, 39}, C64_KEYCODE_1},
      {{39, 50, 39}, C64_KEYCODE_2},     {{39, 25, 39}, C64_KEYCODE_Y},
      {{39, 14, 39}, C64_KEYCODE_N},     {{32, 6, 49}, C64_KEYCODE_F1},
      {{32, 6, 51}, C64_KEYCODE_F3},     {{32, 6, 53}, C64_KEYCODE_F5},
      {{32, 6, 55}, C64_KEYCODE_F7}};
  uint8_t listInGameKeycodesIdx;

  uint8_t getDC01(uint8_t dc00, bool xchgports);
  inline void adaptVICBaseAddrs(bool fromcia) __attribute__((always_inline));
  inline void decodeRegister1(uint8_t val) __attribute__((always_inline));
  inline void checkciatimers(uint8_t cycles) __attribute__((always_inline));
  inline void logDebugInfo() __attribute__((always_inline));
  TextKeycode getNextKeycode();
  void check4extcmd();

public:
  VIC vic;
  CIA cia1;
  CIA cia2;
  SID sid;
  Floppy floppy;
  ExternalCmds *externalCmds;
  Hooks *hooks;
  KeyboardDriver *keyboard;

  C64Sys() : cia1(true), cia2(false), floppy(this) {}

  uint8_t getA();
  void setA(uint8_t ap);
  uint8_t getX();
  void setX(uint8_t xp);
  uint8_t getY();
  void setY(uint8_t yp);
  uint8_t getSP();
  uint8_t getSR();
  uint16_t getPC();

  std::atomic<uint32_t> numofcyclespersecond;
  std::atomic<uint32_t> numofburnedcyclespersecond;
  std::atomic<bool> perf;
  std::atomic<uint16_t> batteryVoltage;
  std::atomic<bool> poweroff;

  // set by class ExternalCmds
  uint8_t joystickmode;
  uint8_t kbjoystickmode;
  bool deactivateTemp;
  bool debug;
  uint16_t debugstartaddr;
  long debugNumOfSteps;
  bool detectreleasekey;

  bool restorenmi;

  uint8_t getMem(uint16_t addr);
  void setMem(uint16_t addr, uint8_t val);

  void cmd6502brk() override;
  void cmd6502halt() override;
  void run() override;

  void startLogCPUCmds(const long numOfCmds) override;

  void initMemAndRegs();
  void init(uint8_t *ram, const uint8_t *charrom);
  void setPC(uint16_t pc);
  void exeSubroutine(uint16_t addr, uint8_t rega, uint8_t regx, uint8_t regy);
  void exeSubroutine(uint16_t regpc);
  void setKeycodes(uint8_t keycode1, uint8_t keycode2);
};

#endif // C64SYS_H
