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
#ifndef TERMINALTEXTKB_H
#define TERMINALTEXTKB_H

#include "../Config.h"
#ifdef USE_TERMINALTEXT_KEYBOARD

#include "../ExtCmd.h"
#include "KeyboardDriver.h"
#include "TerminalTextKeyboardLayout.h"
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>

class TerminalTextKB : public KeyboardDriver {
private:
  std::atomic<uint8_t> kbcode1;
  std::atomic<uint8_t> kbcode2;
  std::atomic<uint8_t> shiftctrlcode;
  std::unique_ptr<TerminalTextKeyboardLayout> keyboardLayout;

  ExtCmd kbjoystickmode = ExtCmd::KBJOYSTICKMODEOFF;
  ExtCmd joystickmode = ExtCmd::JOYSTICKMODEOFF;
  bool joystickActive = false;
  bool kbKeyUp = false;
  bool kbKeyDown = false;
  bool kbKeyLeft = false;
  bool kbKeyRight = false;
  bool kbKeyFire = false;
  bool ctrlDown = false;

  uint32_t lastKey = 0;
  bool keyActive = false;
  uint8_t holdCounter = 0;

  void setCodes(uint8_t code1, uint8_t code2, uint8_t ctrlcode);
  std::optional<CodeTripleS> getKeyCodes(uint32_t key, bool shift, bool alt);
  void handleKeyEvent(uint32_t keycode, bool shift, bool ctrl, bool alt,
                      bool pressed);
  uint32_t readKey();

public:
  TerminalTextKB();
  ~TerminalTextKB() override;

  void init() override;
  void scanKeyboard() override;
  uint8_t getKBCodeDC01() override;
  uint8_t getKBCodeDC00() override;
  uint8_t getShiftctrlcode() override;
  uint8_t getKBJoyValue() override;
};

#endif
#endif // TERMINALTEXTKB_H