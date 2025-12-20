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
#ifndef PLATFORMWINDOWS_H
#define PLATFORMWINDOWS_H

#ifdef _WIN32
#include "Platform.h"
#define SID_DEFINED
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <functional>
#include <random>
#include <thread>
#include <windows.h>

class PlatformWindows : public Platform {
public:
  PlatformWindows() { timeBeginPeriod(1); }

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
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return static_cast<int64_t>((counter.QuadPart * 1000000LL) / freq.QuadPart);
  }

  void waitUS(uint32_t us) override {
#ifdef WINDOWS_BUSYWAIT
    auto target_time =
        std::chrono::steady_clock::now() + std::chrono::microseconds(us);
    while (std::chrono::steady_clock::now() < target_time) {
    }
#else
    std::this_thread::sleep_for(std::chrono::microseconds(us));
#endif
  }

  void waitMS(uint32_t ms) override { Sleep(ms); }

  void feedWDT() override {
    // not needed on Windows
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

  ~PlatformWindows() { timeEndPeriod(1); }
};
#endif

#endif // PLATFORMWINDOWS_H
