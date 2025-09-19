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
#ifndef PLATFORMFACTORY_H
#define PLATFORMFACTORY_H

#include "Platform.h"
#if defined(ESP_PLATFORM)
#include "PlatformESP32.h"
#elif defined(PLATFORM_LINUX)
#include "PlatformLinux.h"
#elif defined(_WIN32)
#include "PlatformWindows.h"
#else
#error "no valid platform defined"
#endif

namespace PlatformNS {
Platform *create() {
#if defined(ESP_PLATFORM)
  return new PlatformESP32();
#elif defined(PLATFORM_LINUX)
  return new PlatformLinux();
#elif defined(_WIN32)
  return new PlatformWindows();
#endif
}
} // namespace PlatformNS

#endif // PLATFORMFACTORY_H
