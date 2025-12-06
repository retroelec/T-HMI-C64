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
#ifndef PLATFORM_H
#define PLATFORM_H

#include <cstdint>
#include <functional>

#ifdef ESP_PLATFORM
#define PLATFORM_ATTR_ISR IRAM_ATTR
#else
#define PLATFORM_ATTR_ISR
#endif

/**
 * @brief Log message severity levels.
 */
enum LogLevel { LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_VERBOSE };

/**
 * @brief Interface for platform-dependent functionality.
 *
 * This interface abstracts hardware or OS-specific operations such as logging,
 * random number generation, timing, and task management. It allows
 * platform-independent components to use system services without direct
 * dependencies.
 */
class Platform {
public:
  /**
   * @brief Logs a formatted message with a given severity level and tag.
   *
   * @param level Log severity level.
   * @param tag Short identifier for the source of the log message.
   * @param format printf-style format string.
   * @param ... Arguments corresponding to the format string.
   */
  virtual void log(LogLevel level, const char *tag, const char *format,
                   ...) = 0;
  /**
   * @brief Returns a single random byte.
   *
   * @return A random 8-bit unsigned integer.
   */
  virtual uint8_t getRandomByte() = 0;

  /**
   * @brief Returns the current time in microseconds.
   *
   * The time base may vary by platform (e.g., time since boot).
   *
   * @return Current time in microseconds.
   */
  virtual int64_t getTimeUS() = 0;

  /**
   * @brief Waits for the specified number of microseconds.
   *
   * @param us Duration to wait in microseconds.
   */
  virtual void waitUS(uint32_t us) = 0;

  /**
   * @brief Waits for the specified number of milliseconds.
   *
   * @param ms Duration to wait in milliseconds.
   */
  virtual void waitMS(uint32_t ms) = 0;

  /**
   * @brief Feeds the system watchdog to prevent a timeout reset.
   *
   */
  virtual void feedWDT() = 0;

  /**
   * @brief Starts a periodic timer that invokes a callback at a fixed interval.
   *
   * @param fn Callback function to be called periodically.
   * @param interval_us Timer interval in microseconds.
   */
  virtual void startIntervalTimer(std::function<void()> fn,
                                  uint64_t interval_us) = 0;

  /**
   * @brief Starts a new task on a specified CPU core with given priority.
   *
   * @param fn Task entry function.
   * @param core CPU core number (e.g., 0 or 1 on ESP32).
   * @param prio Task priority.
   */
  virtual void startTask(std::function<void(void *)> fn, uint8_t core,
                         uint8_t prio) = 0;

  virtual ~Platform(){};
};

#endif // PLATFORM_H
