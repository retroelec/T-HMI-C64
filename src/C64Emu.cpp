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
#include "HardwareInitializationException.h"
#include "OSUtils.h"
#include "VIC.h"
#include "roms/charset.h"
#include <cstdint>
#include <driver/gpio.h>

static const char *TAG = "C64Emu";

C64Emu *C64Emu::instance = nullptr;

void IRAM_ATTR C64Emu::interruptProfilingBatteryCheckFunc() {
  // battery check
  cntSecondsForBatteryCheck++;
  if (cntSecondsForBatteryCheck >= 300) { // each 5 minutes
    // get battery voltage
    int raw_value = 0;
    if (configBoard.boardDriver->getAdcHandle()) {
      adc_oneshot_read(configBoard.boardDriver->getAdcHandle(), Config::BAT_ADC,
                       &raw_value);
      int voltage = 0;
      if (configBoard.boardDriver->getAdcCaliHandle()) {
        adc_cali_raw_to_voltage(configBoard.boardDriver->getAdcCaliHandle(),
                                raw_value, &voltage);
        voltage *= 2;
      } else {
        // fallback if calibration was not successful
        voltage = raw_value * 2;
      }
      cpu.batteryVoltage.store(voltage, std::memory_order_release);
      // if battery voltage is too low, then power off device
      if (cpu.batteryVoltage.load(std::memory_order_acquire) < 3550) {
        configBoard.boardDriver->powerOff();
      }
    }
    // reset "timer"
    cntSecondsForBatteryCheck = 0;
  }
  // power off?
  if (cpu.poweroff.load(std::memory_order_acquire)) {
    configBoard.boardDriver->powerOff();
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

void IRAM_ATTR C64Emu::interruptTODFunc() {
  cpu.cia1.updateTOD();
  cpu.cia2.updateTOD();
}

void IRAM_ATTR C64Emu::interruptScanKeyboardFunc() {
  cpu.configKeyboard.keyboardDriver->scanKeyboard();
}

void C64Emu::cpuCode(void *parameter) {
  OSUtils::log(LOG_INFO, TAG, "cpuTask running on core %d", xPortGetCoreID());

  // interrupt each 8 ms to scan keyboard
  interruptScanKeyboard = timerBegin(1000000);
  timerAttachInterrupt(interruptScanKeyboard,
                       &interruptScanKeyboardFuncWrapper);
  timerAlarm(interruptScanKeyboard, 8000, true, 0);

  // interrupt each 100 ms to increment CIA real time clock (TOD)
  interruptTOD = timerBegin(1000000);
  timerAttachInterrupt(interruptTOD, &C64Emu::interruptTODFuncWrapper);
  timerAlarm(interruptTOD, 100000, true, 0);

  cpu.run();
  // cpu runs forever -> no vTaskDelete(NULL);
}

void C64Emu::setup() {
  instance = this;

  // init board
  configBoard.boardDriver->init();

  // init. instance variables
  cntSecondsForBatteryCheck =
      295; // wait 5 seconds for first battery measurement

  // allocate ram
  ram = new uint8_t[1 << 16];

  // init VIC
  vic.init(ram, charset_rom);
  // init LCD driver
  vic.initLCDController();

  // init CPU
  cpu.init(ram, charset_rom, &vic);

  // start cpu task
  xTaskCreatePinnedToCore(cpuCodeWrapper, "CPU", 10000, NULL, 19, &cpuTask, 1);

  // profiling + battery check: interrupt each second
  interruptProfilingBatteryCheck = timerBegin(1000000);
  timerAttachInterrupt(interruptProfilingBatteryCheck,
                       &interruptProfilingBatteryCheckFuncWrapper);
  timerAlarm(interruptProfilingBatteryCheck, 1000000, true, 0);
}

void C64Emu::loop() {
  vic.refresh();
  vTaskDelay(Config::REFRESHDELAY);
  if (cpu.perf.load(std::memory_order_acquire) &&
      showperfvalues.load(std::memory_order_acquire)) {
    showperfvalues.store(false, std::memory_order_release);
    // frames per second
    if (cntRefreshs.load(std::memory_order_acquire) != 0) {
      OSUtils::log(LOG_INFO, TAG, "fps: %d",
                   cntRefreshs.load(std::memory_order_acquire));
    }
    cntRefreshs.store(0, std::memory_order_release);
    // number of cycles per second
    OSUtils::log(LOG_INFO, TAG, "noc: %lu, nbc: %lu",
                 numofcyclespersecond.load(std::memory_order_acquire),
                 numofburnedcyclespersecond.load(std::memory_order_acquire));
  }
}
