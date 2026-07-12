/*
  Copyright (C) 2024-2026 retroelec <retroelec42@gmail.com>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 3 of the License, or (at your
  option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  For the complete text of the GNU General Public License see
  http://www.gnu.org/licenses/.
*/
#ifndef NOTCURSESKB_H
#define NOTCURSESKB_H

#include "../Config.h"
#ifdef USE_NOTCURSES_KEYBOARD

#include "../ExtCmd.h"
#include "KeyboardDriver.h"
#include "NotcursesKeyboardLayout.h"
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>

class NotcursesKB : public KeyboardDriver {
private:
  uint8_t kbcode1;
  uint8_t kbcode2;
  uint8_t shiftctrlcode;
  std::unique_ptr<NotcursesKeyboardLayout> keyboardLayout;

  ExtCmd kbjoystickmode = ExtCmd::KBJOYSTICKMODEOFF;
  ExtCmd joystickmode = ExtCmd::JOYSTICKMODEOFF;
  bool joystickActive = false;
  bool kbKeyUp = false;
  bool kbKeyDown = false;
  bool kbKeyLeft = false;
  bool kbKeyRight = false;
  bool kbKeyFire = false;
  bool ctrlDown = false;

  void setCodes(uint8_t code1, uint8_t code2, uint8_t ctrlcode);
  std::optional<CodeTripleS> getKeyCodes(uint32_t key, bool shift, bool alt);
  void handleKeyEvent(uint32_t keycode, bool shift, bool ctrl, bool alt,
                      bool pressed);

public:
  NotcursesKB();
  ~NotcursesKB() override;

  void init() override;
  void scanKeyboard() override;
  uint8_t getKBCodeDC01() override;
  uint8_t getKBCodeDC00() override;
  uint8_t getShiftctrlcode() override;
  uint8_t getKBJoyValue() override;
};

#endif
#endif // NOTCURSESKB_H