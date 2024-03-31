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
#include "roms/basic.h"
#include "roms/charset.h"
#include "roms/kernal.h"
#include <ArduinoLog.h>

static const uint8_t INTERRUPTKERNALRESOLUTION = 100;

// memory used in both cores is allocated in Main::setup()
uint8_t *ram;
uint16_t *bitmapmem;
uint8_t *kbbuffer;

CPUC64 cpu;
VIC vic;
CIA cia1;
CIA cia2;
BLEKB blekb;

bool checkExternalCommand = false;
uint16_t checkExternalCommandCnt = 0;
ExternalCmds externalCmds;

hw_timer_t *interruptProfiling = NULL;
hw_timer_t *interruptKernal = NULL;
TaskHandle_t cpuTask;

void IRAM_ATTR interruptProfilingFunc() {
  if (vic.cntRefreshs != 0) {
    Log.noticeln("fps = %d", vic.cntRefreshs);
  }
  vic.cntRefreshs = 0;
  Log.noticeln("noc = %d", cpu.numofcyclespersecond);
  cpu.numofcyclespersecond = 0;
}

void IRAM_ATTR interruptKernalFunc() {
  // check for host commands and keyboard inputs each 16.66ms
  if (externalCmds.hostcmdcode == ExternalCmds::NOHOSTCMD) {
    checkExternalCommandCnt++;
    if (checkExternalCommandCnt == (16667 / INTERRUPTKERNALRESOLUTION)) {
      checkExternalCommand = true;
      checkExternalCommandCnt = 0;
      externalCmds.hostcmdcode = (ExternalCmds::cmds)blekb.getKBCode();
      if (externalCmds.hostcmdcode != ExternalCmds::NOHOSTCMD) {
        return;
      }
    }
  }
  // CIA 1 Timer A
  if (cia1.checkTimerA(INTERRUPTKERNALRESOLUTION)) {
    cpu.irq.store(true, std::memory_order_release);
  }
  // CIA 1 Timer B
  if (cia1.checkTimerB(INTERRUPTKERNALRESOLUTION)) {
    cpu.irq.store(true, std::memory_order_release);
  }
}

void cpuCode(void *parameter) {
  Log.noticeln("cpuTask running on core %d", xPortGetCoreID());
  cpu.run();
  // cpu runs forever -> no vTaskDelete(NULL);
}

void Main::setup() {
  // allocate ram
  ram = (uint8_t *)malloc(1 << 16);
  if (ram == nullptr) {
    Log.noticeln("could not allocate ram");
    while (true) {
    }
  }

  // allocate bitmap memory to be transfered to LCD
  // (consider xscroll and yscroll offset)
  bitmapmem = (uint16_t *)calloc(320 * (200 + 7) + 7, sizeof(uint16_t));
  if (bitmapmem == nullptr) {
    Log.noticeln("could not allocate bitmap memory");
    while (true) {
    }
  }

  // init VIC
  if (!vic.init(ram, charset_rom, bitmapmem)) {
    Log.noticeln("error in init. of VIC");
    while (true) {
    }
  }

  // init joystick
  esp_err_t err = Joystick::init();
  if (err != ESP_OK) {
    Log.noticeln("error in Joystick::init(): %s", esp_err_to_name(err));
    while (true) {
    }
  }

  // init ble keyboard
  kbbuffer = (uint8_t *)malloc(256);
  if (kbbuffer == nullptr) {
    Log.noticeln("could not allocate KB buffer");
    while (true) {
    }
  }
  blekb.init(Config::SERVICE_UUID, Config::CHARACTERISTIC_UUID, kbbuffer);

  // init CIAs
  cia1.init();
  cia2.init();

  // init CPU
  cpu.init(ram, basic_rom, kernal_rom, charset_rom, &vic, &cia1, &cia2, &blekb);
  xTaskCreatePinnedToCore(cpuCode,  // Function to implement the task
                          "CPU",    // Name of the task
                          10000,    // Stack size in words
                          NULL,     // Task input parameter
                          19,        // Priority of the task
                          &cpuTask, // Task handle
                          1);       // Core where the task should run

  // interrupt each INTERRUPTKERNALRESOLUTION us -> standard kernal routine
  // each 16.666 ms
  interruptKernal = timerBegin(1, 80, true);
  timerAttachInterrupt(interruptKernal, &interruptKernalFunc, true);
  timerAlarmWrite(interruptKernal, INTERRUPTKERNALRESOLUTION, true);
  timerAlarmEnable(interruptKernal);

  // profiling: interrupt each second
  /**/
  interruptProfiling = timerBegin(2, 80, true);
  timerAttachInterrupt(interruptProfiling, &interruptProfilingFunc, true);
  timerAlarmWrite(interruptProfiling, 1000000, true);
  timerAlarmEnable(interruptProfiling);
  /**/

  // init ExternalCmds
  externalCmds.init(ram, &cpu, &vic, &cia1, &cia2, &blekb);
}

void Main::loop() {
  if (checkExternalCommand) {
    checkExternalCommand = false;
    externalCmds.checkExternalCmd();
  }
  vic.refresh();
  vTaskDelay(1);
}
