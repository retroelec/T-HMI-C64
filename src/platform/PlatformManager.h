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
