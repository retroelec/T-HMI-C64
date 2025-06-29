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
#ifndef OSUTILS_H
#define OSUTILS_H

#undef CONFIG_LOG_MAXIMUM_LEVEL
#define CONFIG_LOG_MAXIMUM_LEVEL 5

#include <cstdarg>
#include <cstdint>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>

enum LogLevel { LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_VERBOSE };

class OSUtils {
public:
  static void log(LogLevel level, const char *tag, const char *format, ...);
  static uint8_t getRandomByte();
  static int64_t getTimeUS();
  static void pauseExecutionUS(int64_t pause);
  static uint32_t getCPUCycleCount();

  adc_oneshot_unit_handle_t adc_handle;
  adc_cali_handle_t adc_cali_handle;
  void calibrateBattery();
};
#endif // OSUTILS_H
