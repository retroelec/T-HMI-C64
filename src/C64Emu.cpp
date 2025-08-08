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
#include "C64Emu.h"
#include "Config.h"
#include "VIC.h"
#include "board/BoardFactory.h"
#include "platform/PlatformFactory.h"
#include "platform/PlatformManager.h"
#include "roms/charset.h"
#include <cstdint>

static const char *TAG = "C64Emu";

void PLATFORM_ATTR_ISR C64Emu::intervalTimerProfilingBatteryCheckFunc() {
  // battery check
  cntSecondsForBatteryCheck++;
  if (cntSecondsForBatteryCheck >= 300) { // each 5 minutes
    // get battery voltage
    uint16_t voltage = board->getBatteryVoltage();
    cpu.batteryVoltage.store(voltage, std::memory_order_release);
    // if battery voltage is too low, then power off device
    if (voltage < 3400) {
      board->powerOff();
    }
    // reset "timer"
    cntSecondsForBatteryCheck = 0;
  }
  // power off?
  if (cpu.poweroff.load(std::memory_order_acquire)) {
    board->powerOff();
  }
  // profiling (if activated)
  if (!cpu.perf.load(std::memory_order_acquire)) {
    return;
  }
  // frames per second
  if (vic.cntRefreshs.load(std::memory_order_acquire) != 0) {
    cntRefreshs.store(vic.cntRefreshs.load(std::memory_order_acquire),
                      std::memory_order_release);
  }
  vic.cntRefreshs.store(0, std::memory_order_release);
  // number of cycles per second
  numofcyclespersecond.store(
      cpu.numofcyclespersecond.load(std::memory_order_acquire),
      std::memory_order_release);
  numofburnedcyclespersecond.store(
      cpu.numofburnedcyclespersecond.load(std::memory_order_acquire),
      std::memory_order_release);
  cpu.numofcyclespersecond.store(0, std::memory_order_release);
  cpu.numofburnedcyclespersecond.store(0, std::memory_order_release);
  showperfvalues.store(true, std::memory_order_release);
}

void PLATFORM_ATTR_ISR C64Emu::intervalTimerTODFunc() {
  cpu.cia1.updateTOD();
  cpu.cia2.updateTOD();
}

void PLATFORM_ATTR_ISR C64Emu::intervalTimerScanKeyboardFunc() {
  cpu.keyboard->scanKeyboard();
}

void C64Emu::cpuCode(void *parameter) {
  // timer interrupts each 8 ms to scan keyboard
  PlatformManager::getInstance().startIntervalTimer(
      std::bind(&C64Emu::intervalTimerScanKeyboardFunc, this), 8000);

  // timer interrupts each 100 ms to increment CIA real time clock (TOD)
  PlatformManager::getInstance().startIntervalTimer(
      std::bind(&C64Emu::intervalTimerTODFunc, this), 100000);

  cpu.run();
  // cpu runs forever -> no vTaskDelete(NULL);
}

void C64Emu::setup() {
  // init platform
  PlatformManager::initialize(PlatformNS::create());
  PlatformManager::getInstance().log(LOG_INFO, TAG, "start setup...");

  // init board
  board = Board::create();
  board->init();

  // init. instance variables
  cntSecondsForBatteryCheck =
      295; // wait 5 seconds for first battery measurement

  // allocate ram
  ram = new uint8_t[1 << 16];

  // init VIC
  vic.init(ram, charset_rom);

  // init CPU
  cpu.init(ram, charset_rom, &vic);

  // start cpu task
  using namespace std::placeholders;
  PlatformManager::getInstance().startTask(
      std::bind(&C64Emu::cpuCode, this, _1), 1, 19);

  // profiling + battery check: timer interrupts each second
  // using namespace std::placeholders;
  PlatformManager::getInstance().startIntervalTimer(
      std::bind(&C64Emu::intervalTimerProfilingBatteryCheckFunc, this),
      1000000);
}

void C64Emu::loop() {
  vic.refresh();
  PlatformManager::getInstance().feedWDT();
  PlatformManager::getInstance().waitMS(Config::REFRESHDELAY);
  if (cpu.perf.load(std::memory_order_acquire) &&
      showperfvalues.load(std::memory_order_acquire)) {
    showperfvalues.store(false, std::memory_order_release);
    // frames per second
    if (cntRefreshs.load(std::memory_order_acquire) != 0) {
      PlatformManager::getInstance().log(
          LOG_INFO, TAG, "fps: %d",
          cntRefreshs.load(std::memory_order_acquire));
    }
    cntRefreshs.store(0, std::memory_order_release);
    // number of cycles per second
    PlatformManager::getInstance().log(
        LOG_INFO, TAG, "noc: %lu, nbc: %lu",
        numofcyclespersecond.load(std::memory_order_acquire),
        numofburnedcyclespersecond.load(std::memory_order_acquire));
    PlatformManager::getInstance().log(
        LOG_INFO, TAG, "voltage: %d",
        cpu.batteryVoltage.load(std::memory_order_acquire));
  }
}
