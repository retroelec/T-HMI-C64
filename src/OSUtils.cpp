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
#include "OSUtils.h"
#include "Config.h"
#include <cstdarg>
#include <cstdint>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_cpu.h>
#include <esp_random.h>
#include <esp_timer.h>

static const char *TAG = "OSUtils";

void OSUtils::log(LogLevel level, const char *tag, const char *format, ...) {
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

uint8_t OSUtils::getRandomByte() { return (uint8_t)(esp_random() & 0xff); }

int64_t OSUtils::getTimeUS() { return esp_timer_get_time(); }

void OSUtils::pauseExecutionUS(int64_t pause) {
  int64_t now = esp_timer_get_time();
  if (pause > 0) {
    while ((esp_timer_get_time() - now) <= pause) {
    }
  }
}

uint32_t OSUtils::getCPUCycleCount() { return esp_cpu_get_cycle_count(); }

void OSUtils::calibrateBattery() {
  // initialize ADC unit 1 in one-shot mode with default RTC clock source
  adc_oneshot_unit_init_cfg_t init_config = {.unit_id = ADC_UNIT_1,
                                             .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
                                             .ulp_mode = ADC_ULP_MODE_DISABLE};
  adc_oneshot_new_unit(&init_config, &adc_handle);
  // configure the selected ADC channel (for battery voltage measurement)
  // - attenuation of 12 dB allows input voltages up to ~3.3V
  // - default bit width (typically 12 bits)
  adc_oneshot_chan_cfg_t config = {.atten = ADC_ATTEN_DB_12,
                                   .bitwidth = ADC_BITWIDTH_DEFAULT};
  adc_oneshot_config_channel(adc_handle, Config::BAT_ADC, &config);
  // configure ADC calibration using curve fitting scheme
  // this uses calibration data (if available) stored in eFuses
  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = ADC_UNIT_1,
      .chan = Config::BAT_ADC,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
  };
  esp_err_t ret =
      adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle);
  // check if calibration was successful; log error if it failed
  if (ret != ESP_OK) {
    adc_cali_handle = nullptr;
    log(LOG_ERROR, TAG, "adc calibration failed");
  }
}
