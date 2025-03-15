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
#include "C64Emu.h"
#include "Config.h"
#include "HardwareInitializationException.h"
#include "VIC.h"
#include "roms/charset.h"
#include <cstdint>
#include <driver/gpio.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>

static const char *TAG = "C64Emu";

C64Emu *C64Emu::instance = nullptr;

void IRAM_ATTR C64Emu::interruptProfilingBatteryCheckFunc() {
  // battery check
  cntSecondsForBatteryCheck++;
  if (cntSecondsForBatteryCheck >= 300) { // each 5 minutes
    // get battery voltage
    int raw_value = 0;
    adc_oneshot_read(adc1_handle, Config::BAT_ADC, &raw_value);
    int voltage = 0;
    if (adc_cali_handle) {
      adc_cali_raw_to_voltage(adc_cali_handle, raw_value, &voltage);
      voltage *= 2;
    } else {
      // fallback if calibration was not successful
      voltage = raw_value * 2;
    }
    batteryVoltage = voltage;
    ESP_LOGI(TAG, "adc raw: %d", raw_value);
    ESP_LOGI(TAG, "battery voltage: %d mV", batteryVoltage);
    // if battery voltage is too low, then power off device
    if (batteryVoltage < 3550) {
      powerOff();
    }
    // reset "timer"
    cntSecondsForBatteryCheck = 0;
  }

  // profiling (if activated)
  if (!perf) {
    return;
  }
  // frames per second
  if (vic.cntRefreshs != 0) {
    ESP_LOGI(TAG, "fps: %d", vic.cntRefreshs);
  }
  vic.cntRefreshs = 0;
  // number of cycles per second
  ESP_LOGI(TAG, "noc: %d, nbc: %d", cpu.numofcyclespersecond,
           numofburnedcyclespersecond);
  cpu.numofcyclespersecond = 0;
  numofburnedcyclespersecond = 0;
}

bool C64Emu::updateTOD(CIA &cia) {
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

void IRAM_ATTR C64Emu::interruptTODFunc() {
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

void IRAM_ATTR C64Emu::interruptSystemFunc() {
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

void C64Emu::cpuCode(void *parameter) {
  ESP_LOGI(TAG, "cpuTask running on core %d", xPortGetCoreID());

  // init LCD driver
  vic.initLCDController();

  // interrupt each 100 ms to increment CIA real time clock (TOD)
  interruptTOD = timerBegin(1000000);
  timerAttachInterrupt(interruptTOD, &C64Emu::interruptTODFuncWrapper);
  timerAlarm(interruptTOD, 100000, true, 0);

  cpu.run();
  // cpu runs forever -> no vTaskDelete(NULL);
}

void C64Emu::powerOff() {
#ifdef BOARD_T_HMI
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << Config::PWR_ON);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);
  gpio_set_level((gpio_num_t)Config::PWR_ON, 0);
#endif
}

void C64Emu::setup() {
  instance = this;

  // init board
  configBoard.boardDriver->init();
  adc_oneshot_unit_init_cfg_t init_config = {.unit_id = ADC_UNIT_1,
                                             .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
                                             .ulp_mode = ADC_ULP_MODE_DISABLE};
  adc_oneshot_new_unit(&init_config, &adc1_handle);
  adc_oneshot_chan_cfg_t config = {.atten = ADC_ATTEN_DB_12,
                                   .bitwidth = ADC_BITWIDTH_DEFAULT};
  adc_oneshot_config_channel(adc1_handle, Config::BAT_ADC, &config);
  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = ADC_UNIT_1,
      .chan = Config::BAT_ADC,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
  };
  esp_err_t ret =
      adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "adc calibration failed");
  }

  // init. instance variables
  cntSecondsForBatteryCheck =
      295; // wait 5 seconds for first battery measurement

  // allocate ram
  ram = new uint8_t[1 << 16];

  // init VIC
  vic.init(ram, charset_rom);

  // init ble keyboard
  blekb.init(this);

  // init CPU
  cpu.init(ram, charset_rom, &vic, this);

  // init ExternalCmds (must be initialized after cpu!)
  externalCmds.init(ram, this);

  // start cpu task
  xTaskCreatePinnedToCore(cpuCodeWrapper, // Function to implement the task
                          "CPU",          // Name of the task
                          10000,          // Stack size in words
                          NULL,           // Task input parameter
                          19,             // Priority of the task
                          &cpuTask,       // Task handle
                          1);             // Core where the task should run

  // interrupt each 1000 us to get keyboard codes and throttle 6502 CPU
  interruptSystem = timerBegin(1000000);
  timerAttachInterrupt(interruptSystem, &interruptSystemFuncWrapper);
  timerAlarm(interruptSystem, Config::INTERRUPTSYSTEMRESOLUTION, true, 0);

  // profiling + battery check: interrupt each second
  interruptProfilingBatteryCheck = timerBegin(1000000);
  timerAttachInterrupt(interruptProfilingBatteryCheck,
                       &interruptProfilingBatteryCheckFuncWrapper);
  timerAlarm(interruptProfilingBatteryCheck, 1000000, true, 0);
}

void C64Emu::loop() {
  vic.refresh();
  vTaskDelay(Config::REFRESHDELAY);
}
