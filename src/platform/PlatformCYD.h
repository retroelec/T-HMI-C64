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
#ifndef PLATFORMCYD_H
#define PLATFORMCYD_H

#include "../Config.h"
#if defined(BOARD_CYD)
#include "PlatformESP32.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class PlatformCYD : public PlatformESP32 {
private:
  SemaphoreHandle_t mutex;

public:
  PlatformCYD() { mutex = xSemaphoreCreateMutex(); }

  bool lock(TickType_t wait) override {
    return xSemaphoreTake(mutex, wait) == pdTRUE;
  }

  void unlock() override { xSemaphoreGive(mutex); }
};
#endif

#endif // PLATFORMCYD_H
