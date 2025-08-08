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
#ifndef PLATFORMLINUX_H
#define PLATFORMLINUX_H

#ifdef PLATFORM_LINUX
#include "Platform.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <random>
#include <thread>
#include <time.h>
#include <unistd.h>

class PlatformLinux : public Platform {
public:
  PlatformLinux() = default;

  void log(LogLevel level, const char *tag, const char *format, ...) override {
    static const char *levelStrs[] = {"[E]", "[W]", "[I]", "[D]", "[V]"};
    va_list args;
    va_start(args, format);
    std::fprintf(stderr, "%s[%s] ", levelStrs[level], tag);
    std::vfprintf(stderr, format, args);
    std::fprintf(stderr, "\n");
    va_end(args);
  }

  uint8_t getRandomByte() override {
    static std::random_device rd;
    static std::mt19937 rng(rd());
    static std::uniform_int_distribution<int> dist(0, 255);
    return static_cast<uint8_t>(dist(rng));
  }

  int64_t getTimeUS() override {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
  }

  void waitUS(uint32_t us) override { usleep(us); }

  void waitMS(uint32_t ms) override {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }

  void feedWDT() override {
    // not needed on Linux
  }

  void startIntervalTimer(std::function<void()> fn,
                          uint64_t interval_us) override {
    std::thread([fn, interval_us]() {
      while (true) {
        std::this_thread::sleep_for(std::chrono::microseconds(interval_us));
        fn();
      }
    }).detach();
  }

  void startTask(std::function<void(void *)> fn, uint8_t /*core*/,
                 uint8_t /*prio*/) override {
    std::thread([fn]() { fn(nullptr); }).detach();
  }

  ~PlatformLinux() override = default;
};
#endif

#endif // PLATFORMLINUX_H
