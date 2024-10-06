/*
 Copyright (C) 2024 retroelec <retroelec42@gmail.com>

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
#include "Main.h"
#include "BLEKB.h"
#include "CPUC64.h"
#include "Config.h"
#include "ExternalCmds.h"
#include "HardwareInitializationException.h"
#include "VIC.h"
#include "roms/charset.h"
#include <cstdint>
#include <esp_log.h>

static const char *TAG = "Main";

uint8_t *ram;
VIC vic;
CPUC64 cpu;
BLEKB blekb;
ExternalCmds externalCmds;

uint16_t checkForKeyboardCnt = 0;

uint8_t throttlingCnt = 0;
uint32_t numofburnedcyclespersecond = 0;

hw_timer_t *interruptProfiling = NULL;
hw_timer_t *interruptTOD = NULL;
hw_timer_t *interruptSystem = NULL;
TaskHandle_t cpuTask;

void IRAM_ATTR interruptProfilingFunc() {
  if (!cpu.perf) {
    return;
  }
  if (vic.cntRefreshs != 0) {
    ESP_LOGI(TAG, "fps: %d", vic.cntRefreshs);
  }
  vic.cntRefreshs = 0;
  ESP_LOGI(TAG, "noc: %d, nbc: %d", cpu.numofcyclespersecond,
           numofburnedcyclespersecond);
  cpu.numofcyclespersecond = 0;
  numofburnedcyclespersecond = 0;
}

bool updateTOD(CIA &cia) {
  uint8_t dc08 = cia.latchrundc08.load(std::memory_order_acquire);
  dc08++;
  if (dc08 > 9) {
    dc08 = 0;
    uint8_t dc09 = cia.latchrundc09.load(std::memory_order_acquire);
    uint8_t dc09one = dc09 & 15;
    uint8_t dc09ten = dc09 >> 4;
    dc09one++;
    if (dc09one > 9) {
      dc09one = 0;
      dc09ten++;
      if (dc09ten > 5) {
        dc09ten = 0;
        uint8_t dc0a = cia.latchrundc0a.load(std::memory_order_acquire);
        uint8_t dc0aone = dc0a & 15;
        uint8_t dc0aten = dc0a >> 4;
        dc0aone++;
        if (dc0aone > 9) {
          dc0aone = 0;
          dc0aten++;
          if (dc0aten > 5) {
            dc0aten = 0;
            uint8_t dc0b = cia.latchrundc0b.load(std::memory_order_acquire);
            uint8_t dc0bone = dc0b & 15;
            uint8_t dc0bten = dc0b >> 4;
            bool pm = dc0b & 128;
            dc0bone++;
            if (((dc0bten == 0) && (dc0bone > 9)) ||
                (dc0bten && (dc0bone > 1))) {
              dc0bone = 0;
              dc0bten++;
              if (dc0bten > 1) {
                dc0bten = 0;
                pm = !pm;
              }
            }
            cia.latchrundc0b.store(dc0bone | (dc0bten << 4) | (pm ? 127 : 0),
                                   std::memory_order_release);
          }
        }
        cia.latchrundc0a.store(dc0aone | (dc0aten << 4),
                               std::memory_order_release);
      }
    }
    cia.latchrundc09.store(dc09one | (dc09ten << 4), std::memory_order_release);
  }
  cia.latchrundc08.store(dc08, std::memory_order_release);
  uint8_t alarmdc08 = cia.latchalarmdc08.load(std::memory_order_acquire);
  if (dc08 == alarmdc08) {
    uint8_t dc09 = cia.latchrundc09.load(std::memory_order_acquire);
    uint8_t alarmdc09 = cia.latchalarmdc09.load(std::memory_order_acquire);
    if (dc09 == alarmdc09) {
      uint8_t dc0a = cia.latchrundc0a.load(std::memory_order_acquire);
      uint8_t alarmdc0a = cia.latchalarmdc0a.load(std::memory_order_acquire);
      if (dc0a == alarmdc0a) {
        uint8_t dc0b = cia.latchrundc0b.load(std::memory_order_acquire);
        uint8_t alarmdc0b = cia.latchalarmdc0b.load(std::memory_order_acquire);
        if (dc0b == alarmdc0b) {
          return true;
        }
      }
    }
  }
  return false;
}

void IRAM_ATTR interruptTODFunc() {
  if (cpu.cia1.isTODRunning.load(std::memory_order_acquire)) {
    if (updateTOD(cpu.cia1)) {
      cpu.cia1.isAlarm.store(true, std::memory_order_release);
    }
  }
  if (cpu.cia2.isTODRunning.load(std::memory_order_acquire)) {
    if (updateTOD(cpu.cia2)) {
      cpu.cia2.isAlarm.store(true, std::memory_order_release);
    }
  }
}

void IRAM_ATTR interruptSystemFunc() {
  // check for keyboard inputs ca. each 8 ms
  checkForKeyboardCnt++;
  if (checkForKeyboardCnt == (8333 / Config::INTERRUPTSYSTEMRESOLUTION)) {
    blekb.handleKeyPress();
    checkForKeyboardCnt = 0;
  }

  // throttle 6502 CPU
  throttlingCnt++;
  uint16_t measuredcyclestmp =
      cpu.measuredcycles.load(std::memory_order_acquire);
  // do not throttle at first half of each "measurement period" (otherwise CPU
  // is throtteled a little bit too much)
  if (throttlingCnt >= Config::THROTTELINGNUMSTEPS / 2) {
    if (measuredcyclestmp > throttlingCnt * Config::INTERRUPTSYSTEMRESOLUTION) {
      uint16_t adjustcycles =
          measuredcyclestmp - throttlingCnt * Config::INTERRUPTSYSTEMRESOLUTION;
      cpu.adjustcycles.store(adjustcycles, std::memory_order_release);
      numofburnedcyclespersecond += adjustcycles;
    }
    if (throttlingCnt == Config::THROTTELINGNUMSTEPS) {
      throttlingCnt = 0;
      cpu.measuredcycles.store(0, std::memory_order_release);
    }
  }
}

void cpuCode(void *parameter) {
  ESP_LOGI(TAG, "cpuTask running on core %d", xPortGetCoreID());

  // init LCD driver
  vic.initLCDController();

  // interrupt each 100 ms to increment CIA real time clock (TOD)
  interruptTOD = timerBegin(1000000);
  timerAttachInterrupt(interruptTOD, &interruptTODFunc);
  timerAlarm(interruptTOD, 100000, true, 0);

  cpu.run();
  // cpu runs forever -> no vTaskDelete(NULL);
}

void Main::setup() {
  // allocate ram
  ram = new uint8_t[1 << 16];

  // init VIC
  vic.init(ram, charset_rom);

  // init ble keyboard
  blekb.init(&externalCmds);

  // init CPU
  cpu.init(ram, charset_rom, &vic, &blekb);

  // init ExternalCmds (must be initialized after cpu!)
  externalCmds.init(ram, &cpu);

  // start cpu task
  xTaskCreatePinnedToCore(cpuCode,  // Function to implement the task
                          "CPU",    // Name of the task
                          10000,    // Stack size in words
                          NULL,     // Task input parameter
                          19,       // Priority of the task
                          &cpuTask, // Task handle
                          1);       // Core where the task should run

  // interrupt each 1000 us to get keyboard codes and throttle 6502 CPU
  interruptSystem = timerBegin(1000000);
  timerAttachInterrupt(interruptSystem, &interruptSystemFunc);
  timerAlarm(interruptSystem, Config::INTERRUPTSYSTEMRESOLUTION, true, 0);

  // profiling: interrupt each second
  interruptProfiling = timerBegin(1000000);
  timerAttachInterrupt(interruptProfiling, &interruptProfilingFunc);
  timerAlarm(interruptProfiling, 1000000, true, 0);
}

void Main::loop() {
  vic.refresh(cpu.refreshframecolor);
  vTaskDelay(Config::REFRESHDELAY);
}
