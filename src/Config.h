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
#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <esp_adc/adc_oneshot.h>

struct Config {

#if defined(BOARD_T_HMI)
#define USE_ST7789V
#define USE_SDCARD
#define USE_JOYSTICK
#define USE_NOSOUND

  static const uint8_t PWR_EN = 10;
  static const uint8_t PWR_ON = 14;
  static const adc_channel_t BAT_ADC = ADC_CHANNEL_4; // GPIO5

  // ST7789V
  static const uint8_t BL = 38;
  static const uint8_t CS = 6;
  static const uint8_t DC = 7;
  static const uint8_t WR = 8;
  static const uint8_t D0 = 48;
  static const uint8_t D1 = 47;
  static const uint8_t D2 = 39;
  static const uint8_t D3 = 40;
  static const uint8_t D4 = 41;
  static const uint8_t D5 = 42;
  static const uint8_t D6 = 45;
  static const uint8_t D7 = 46;

  // DisplayDriver (considering a possible rotation)
  static const uint16_t LCDWIDTH = 320;
  static const uint16_t LCDHEIGHT = 240;
  static const uint8_t REFRESHDELAY = 1;

  // SDCard
  static const uint8_t SD_MISO_PIN = 13;
  static const uint8_t SD_MOSI_PIN = 11;
  static const uint8_t SD_SCLK_PIN = 12;

  // Joystick
  static const adc_channel_t ADC_JOYSTICK_X = ADC_CHANNEL_4;
  static const adc_channel_t ADC_JOYSTICK_Y = ADC_CHANNEL_5;
  static const uint8_t JOYSTICK_FIRE_PIN = 18;
  static const uint8_t JOYSTICK_FIRE2_PIN = 17;

#elif defined(BOARD_T_DISPLAY_S3)
#define USE_RM67162
#define USE_NOSOUND

  static const adc_channel_t BAT_ADC = ADC_CHANNEL_3; // GPIO4

  // DisplayDriver (considering a possible rotation)
  static const uint16_t LCDWIDTH = 536;
  static const uint16_t LCDHEIGHT = 240;
  static const uint8_t REFRESHDELAY = 13;

#elif defined(BOARD_WAVESHARE)
#define USE_ST7789VSERIAL
#define USE_SDCARD
#define USE_JOYSTICK
#define USE_I2SSOUND

  static const adc_channel_t BAT_ADC = ADC_CHANNEL_7; // GPIO8

  // DisplayDriver (considering a possible rotation)
  static const uint16_t LCDWIDTH = 320;
  static const uint16_t LCDHEIGHT = 240;
  static const uint8_t REFRESHDELAY = 8;

  // Sound
  static const uint8_t I2S_DOUT = 47;
  static const uint8_t I2S_BCLK = 48;
  static const uint8_t I2S_LRC = 38;

  // SDCard
  static const uint8_t SD_MISO_PIN = 16;
  static const uint8_t SD_MOSI_PIN = 17;
  static const uint8_t SD_SCLK_PIN = 14;

  // Joystick
  static const adc_channel_t ADC_JOYSTICK_X = ADC_CHANNEL_4;
  static const adc_channel_t ADC_JOYSTICK_Y = ADC_CHANNEL_7;
  static const uint8_t JOYSTICK_FIRE_PIN = 11;
  static const uint8_t JOYSTICK_FIRE2_PIN = 10;

#endif

  // BLEKB
  static constexpr const char *SERVICE_UUID =
      "695ba701-a48c-43f6-9028-3c885771f19f";
  static constexpr const char *CHARACTERISTIC_UUID =
      "3b05e9bf-086f-4b56-9c37-7b7eeb30b28b";

  // resolution of system timer (get BLE KB codes)
  static const uint16_t INTERRUPTSYSTEMRESOLUTION = 1000;

  // audio sample rate
  static const uint16_t AUDIO_SAMPLE_RATE = 44100;
}; // namespace Config

#endif // CONFIG_H
