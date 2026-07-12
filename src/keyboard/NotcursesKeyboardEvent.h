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
#ifndef NOTCURSESKEYBOARDEVENT_H
#define NOTCURSESKEYBOARDEVENT_H

#include "../ExtCmd.h"
#include "CodeTripleDef.h"
#include <cstdint>
#include <functional>
#include <optional>

using GetKeyCodesCallback =
    std::function<std::optional<CodeTripleS>(uint32_t, bool, bool)>;
using SetCodesCallback = std::function<void(uint8_t, uint8_t, uint8_t)>;

void processKeyEvent(uint32_t keycode, bool shift, bool lctrl, bool rctrl,
                     bool lalt, bool ralt, bool pressed,
                     GetKeyCodesCallback getKeyCodes,
                     SetCodesCallback setCodes);

#endif