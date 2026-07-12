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
#ifndef NOTCURSESKEYBOARDFACTORY_H
#define NOTCURSESKEYBOARDFACTORY_H

#include "../Config.h"
#ifdef USE_NOTCURSES_KEYBOARD

#include "NotcursesGermanLayout.h"
#include "NotcursesKeyboardLayout.h"
#include "NotcursesSwissLayout.h"
#include "NotcursesUSLayout.h"
#include <algorithm>
#include <memory>
#include <string>

class NotcursesKeyboardFactory {
public:
  enum class LayoutType { SWISS, GERMAN, US };

  static std::unique_ptr<NotcursesKeyboardLayout> create(LayoutType type) {
    switch (type) {
    case LayoutType::SWISS:
      return std::make_unique<NotcursesSwissLayout>();
    case LayoutType::GERMAN:
      return std::make_unique<NotcursesGermanLayout>();
    case LayoutType::US:
      return std::make_unique<NotcursesUSLayout>();
    default:
      return std::make_unique<NotcursesUSLayout>();
    }
  }

  static std::unique_ptr<NotcursesKeyboardLayout>
  createFromString(std::string name) {
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (name == "ch")
      return create(LayoutType::SWISS);
    if (name == "de")
      return create(LayoutType::GERMAN);
    if (name == "us")
      return create(LayoutType::US);
    return create(LayoutType::US);
  }
};

#endif
#endif