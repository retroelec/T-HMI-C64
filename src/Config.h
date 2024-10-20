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
#include <driver/adc.h>

#define BOARD_T_HMI
//#define BOARD_T_DISPLAY_S3

struct Config {

#if defined(BOARD_T_HMI)
#define USE_ST7789V
#define USE_SDCARD
#define USE_JOYSTICK

  static const uint8_t PWR_EN = 10;
  static const uint8_t PWR_ON = 14;

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
  static const adc2_channel_t ADC_JOYSTICK_X = ADC2_CHANNEL_4;
  static const adc2_channel_t ADC_JOYSTICK_Y = ADC2_CHANNEL_5;
  static const uint8_t JOYSTICK_FIRE_PIN = 18;
  static const uint8_t JOYSTICK_FIRE2_PIN = 17;

#elif defined(BOARD_T_DISPLAY_S3)
#define USE_RM67162

  // DisplayDriver (considering a possible rotation)
  static const uint16_t LCDWIDTH = 536;
  static const uint16_t LCDHEIGHT = 240;
  static const uint8_t REFRESHDELAY = 13;
#endif

  // BLEKB
  static constexpr char *SERVICE_UUID = "695ba701-a48c-43f6-9028-3c885771f19f";
  static constexpr char *CHARACTERISTIC_UUID =
      "3b05e9bf-086f-4b56-9c37-7b7eeb30b28b";

  // resolution of system timer (throttling 6502 CPU, get BLE KB codes)
  static const uint16_t INTERRUPTSYSTEMRESOLUTION = 1000;

  // number of "steps" to average throttling
  static const uint8_t THROTTELINGNUMSTEPS = 50;
}; // namespace Config

#endif // CONFIG_H
