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
CPUC64 cpu;
VIC vic;
BLEKB blekb;

bool checkExternalCommand = false;
uint16_t checkExternalCommandCnt = 0;
ExternalCmds externalCmds;

hw_timer_t *interruptProfiling = NULL;
hw_timer_t *interruptSystem = NULL;
TaskHandle_t cpuTask;

void IRAM_ATTR interruptProfilingFunc() {
  if (vic.cntRefreshs != 0) {
    ESP_LOGI(TAG, "fps: %d", vic.cntRefreshs);
  }
  vic.cntRefreshs = 0;
  ESP_LOGI(TAG, "noc: %d", cpu.numofcyclespersecond);
  cpu.numofcyclespersecond = 0;
}

void IRAM_ATTR interruptSystemFunc() {
  // check for host commands and keyboard inputs each ca. 8 ms
  if (externalCmds.hostcmdcode == ExternalCmds::NOHOSTCMD) {
    checkExternalCommandCnt++;
    if (checkExternalCommandCnt == (8333 / Config::INTERRUPTSYSTEMRESOLUTION)) {
      checkExternalCommand = true;
      checkExternalCommandCnt = 0;
      externalCmds.hostcmdcode = (ExternalCmds::cmds)blekb.getKBCode();
    }
  }
  // throttle 6502 CPU
  uint16_t measuredcyclestmp =
      cpu.measuredcycles.load(std::memory_order_acquire);
  if (measuredcyclestmp > Config::INTERRUPTSYSTEMRESOLUTION) {
    cpu.adjustcycles.store(
        (measuredcyclestmp - Config::INTERRUPTSYSTEMRESOLUTION),
        std::memory_order_release);
  }
  cpu.measuredcycles.store(0, std::memory_order_release);
}

void cpuCode(void *parameter) {
  ESP_LOGI(TAG, "cpuTask running on core %d", xPortGetCoreID());
  cpu.run();
  // cpu runs forever -> no vTaskDelete(NULL);
}

void Main::setup() {
  // allocate ram
  ram = new uint8_t[1 << 16];

  // init VIC
  vic.init(ram, charset_rom);

  // init ble keyboard
  blekb.init();

  // init CPU
  cpu.init(ram, charset_rom, &vic, &blekb);

  // start cpu task
  xTaskCreatePinnedToCore(cpuCode,  // Function to implement the task
                          "CPU",    // Name of the task
                          10000,    // Stack size in words
                          NULL,     // Task input parameter
                          19,       // Priority of the task
                          &cpuTask, // Task handle
                          1);       // Core where the task should run

  // interrupt each 1000 us to get keyboard codes and throttle 6502 CPU
  interruptSystem = timerBegin(1, 80, true);
  timerAttachInterrupt(interruptSystem, &interruptSystemFunc, true);
  timerAlarmWrite(interruptSystem, Config::INTERRUPTSYSTEMRESOLUTION, true);
  timerAlarmEnable(interruptSystem);

  // profiling: interrupt each second
  /*
  interruptProfiling = timerBegin(2, 80, true);
  timerAttachInterrupt(interruptProfiling, &interruptProfilingFunc, true);
  timerAlarmWrite(interruptProfiling, 1000000, true);
  timerAlarmEnable(interruptProfiling);
  */

  // init ExternalCmds
  externalCmds.init(ram, &cpu, &blekb);
}

void Main::loop() {
  if (checkExternalCommand) {
    checkExternalCommand = false;
    externalCmds.checkExternalCmd();
  }
  vic.refresh();
  vTaskDelay(1);
}
