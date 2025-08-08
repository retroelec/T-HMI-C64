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
#ifndef PLATFORMESP32S3_H
#define PLATFORMESP32S3_H

#ifdef ESP_PLATFORM
#include "Platform.h"
#include <cstdarg>
#include <cstdint>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_cpu.h>
#include <esp_random.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <memory>
#include <mutex>

#include <cstdarg>
#include <esp_log.h>
#include <functional>

class PlatformESP32 : public Platform {
private:
  struct TimerContext {
    std::function<void()> func;
  };

  static void IRAM_ATTR genericCallback(void *arg) {
    auto *ctx = static_cast<TimerContext *>(arg);
    if (ctx && ctx->func) {
      ctx->func();
    }
  }

  struct TaskContext {
    std::function<void(void *)> func;
  };

  static void taskEntryPoint(void *arg) {
    std::unique_ptr<TaskContext> ctx(static_cast<TaskContext *>(arg));
    if (ctx && ctx->func) {
      ctx->func(nullptr);
    }
  }

public:
  PlatformESP32() { esp_log_level_set("*", ESP_LOG_INFO); }

  void log(LogLevel level, const char *tag, const char *format, ...) override {
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

  uint8_t getRandomByte() override { return (uint8_t)(esp_random() & 0xff); }

  int64_t getTimeUS() override { return esp_timer_get_time(); }

  void waitUS(uint32_t us) override {
    int64_t now = getTimeUS();
    while ((getTimeUS() - now) <= us) {
    }
  }

  void waitMS(uint32_t ms) override {
    if (ms > 0) {
      vTaskDelay(pdMS_TO_TICKS(ms));
    }
  }

  void feedWDT() override { vTaskDelay(1); }

  void startIntervalTimer(std::function<void()> timerFunction,
                          uint64_t interval_us) override {
    auto *ctx = new TimerContext{timerFunction};
    esp_timer_create_args_t timerArgs = {.callback = &genericCallback,
                                         .arg = ctx,
                                         .dispatch_method = ESP_TIMER_TASK,
                                         .name = "PlatformESP32Timer",
                                         .skip_unhandled_events = false};
    esp_timer_handle_t handle;
    ESP_ERROR_CHECK(esp_timer_create(&timerArgs, &handle));
    ESP_ERROR_CHECK(esp_timer_start_periodic(handle, interval_us));
  }

  void startTask(std::function<void(void *)> taskFunction, uint8_t core,
                 uint8_t prio) override {
    auto *ctx = new TaskContext{std::move(taskFunction)};
    xTaskCreatePinnedToCore(taskEntryPoint, "genericTask", 10000, ctx, prio,
                            nullptr, core);
  }
};
#endif

#endif // PLATFORMESP32S3_H
