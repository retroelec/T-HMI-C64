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
#include "NotcursesKeyboardEvent.h"
#include <optional>

void processKeyEvent(uint32_t keycode, bool shift, bool lctrl, bool rctrl,
                     bool lalt, bool ralt, bool pressed,
                     GetKeyCodesCallback getKeyCodes,
                     SetCodesCallback setCodes) {
  (void)rctrl;
  if (pressed) {
    bool found = false;

    if (ralt) {
      std::optional<CodeTripleS> c64Code = getKeyCodes(keycode, false, true);
      if (c64Code.has_value()) {
        auto [b1, b2, b3] = c64Code.value();
        if (shift) {
          b3 |= 1;
        }
        setCodes(b1, b2, b3);
        found = true;
      }
    }

    if (!found && !lalt) {
      std::optional<CodeTripleS> c64Code = getKeyCodes(keycode, false, false);
      if (c64Code.has_value()) {
        auto [b1, b2, b3] = c64Code.value();
        if (shift) {
          b3 |= 1;
        }
        if (lctrl) {
          b3 |= 2;
        }
        if (lalt || ralt) {
          b3 |= 4;
        }
        setCodes(b1, b2, b3);
        found = true;
      }
    }

    if (!found) {
      setCodes(0xff, 0xff, 0x00);
    }
  } else {
    setCodes(0xff, 0xff, 0x00);
  }
}