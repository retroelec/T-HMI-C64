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
#ifndef PLATFORM_MANAGER_H
#define PLATFORM_MANAGER_H

#include "Platform.h"
#include <stdexcept>

/**
 * @brief Singleton accessor and initializer for the platform interface.
 *
 * This class manages a globally available platform instance.
 */
class PlatformManager {
private:
  static inline Platform *instance = nullptr;

public:
  /**
   * @brief Returns the global platform instance.
   * @throws std::runtime_error if the instance was not initialized.
   */
  static Platform &getInstance() {
    if (!instance) {
      throw std::runtime_error("PlatformManager not initialized");
    }
    return *instance;
  }

  /**
   * @brief Initializes the global platform instance.
   * @param p Pointer to a platform implementation.
   */
  static void initialize(Platform *p) { instance = p; }
};

#endif // PLATFORM_MANAGER_H
