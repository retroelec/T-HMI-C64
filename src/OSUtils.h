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
#include <esp_cpu.h>
#include <esp_log.h>
#include <esp_random.h>
#include <esp_timer.h>

enum LogLevel { LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_VERBOSE };

class OSUtils {
public:
  static void log(LogLevel level, const char *tag, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char msg[256];
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);
    const char *levelStr = "";
    esp_log_level_t espLevel = ESP_LOG_NONE;
    switch (level) {
    case LOG_ERROR:
      levelStr = "E";
      espLevel = ESP_LOG_ERROR;
      break;
    case LOG_WARN:
      levelStr = "W";
      espLevel = ESP_LOG_WARN;
      break;
    case LOG_INFO:
      levelStr = "I";
      espLevel = ESP_LOG_INFO;
      break;
    case LOG_DEBUG:
      levelStr = "D";
      espLevel = ESP_LOG_DEBUG;
      break;
    case LOG_VERBOSE:
      levelStr = "V";
      espLevel = ESP_LOG_VERBOSE;
      break;
    }
    char fullmsg[300];
    snprintf(fullmsg, sizeof(fullmsg), "[%s][%s] %s\n", levelStr, tag, msg);
    esp_log_write(espLevel, tag, fullmsg);
  }

  inline static uint8_t getRandomByte() {
    return (uint8_t)(esp_random() & 0xff);
  }

  inline static int64_t getTimeUS() { return esp_timer_get_time(); }

  inline static void pauseExecutionUS(uint32_t pause) {
    return esp_rom_delay_us(pause);
  }

  inline static uint32_t getCPUCycleCount() {
    return esp_cpu_get_cycle_count();
  }
};
#endif // OSUTILS_H
