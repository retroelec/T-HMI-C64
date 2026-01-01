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
#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#ifdef ESP_PLATFORM
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#endif

#if defined(PLATFORM_LINUX) || defined(_WIN32)

#define BOARD_LINUX
#define USE_SDL_DISPLAY
#define USE_SDL_KEYBOARD
#define USE_LINUXFS
#define USE_SDLJOYSTICK
#define USE_SDLSOUND
#define WINDOWS_BUSYWAIT

#elif defined(ESP_PLATFORM)

// keyboard type (ble, web) is determined in the Makefile
#if defined(BOARD_T_HMI)
#define USE_ST7789V
#define USE_SDCARD
#define USE_ARDUINOJOYSTICK
#define USE_NOSOUND
#elif defined(BOARD_T_DISPLAY_S3)
#define USE_RM67162
#define USE_NOFS
#define USE_NOJOYSTICK
#define USE_NOSOUND
#elif defined(BOARD_WAVESHARE)
#define USE_ST7789VSERIAL
#define USE_SDCARD
#define USE_ARDUINOJOYSTICK
#define USE_I2SSOUND
#endif

#endif

// global defines
#define AUDIO_SAMPLE_RATE 44100

#if defined(PLATFORM_LINUX) || defined(_WIN32)

#define HAS_DEFAULT_VOLUME

struct Config {
  // --- constants to be defined for each board ---

  // delay until next display refresh
  static const uint8_t REFRESHDELAY = 20;

  // "heuristic performance factor"
  static constexpr double HEURISTIC_PERFORMANCE_FACTOR = 1.0;

  // audio
  static const uint8_t DEFAULT_VOLUME = 10;

  // --- driver specific constants ---

  // display driver
  static const uint16_t LCDWIDTH = 404;
  static const uint16_t LCDHEIGHT = 284;
  static inline uint16_t LCDSCALE = 3;

  // filesystem
  static constexpr const char *PATH = "c64prgs/";
  static constexpr const char *CONFIGFILE = ".config.json";
};

#elif defined(ESP_PLATFORM)

#if defined(BOARD_T_HMI)

struct Config {
  // --- constants to be defined for each board ---

  // delay until next display refresh
  static const uint8_t REFRESHDELAY = 0;

  // "heuristic performance factor"
  static constexpr double HEURISTIC_PERFORMANCE_FACTOR = 1.0;

  // --- driver specific constants ---

  // power
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

  // display driver
  static const uint16_t LCDWIDTH = 320;
  static const uint16_t LCDHEIGHT = 240;

  // filesystem
  static constexpr const char *PATH = "";
  static constexpr const char *CONFIGFILE = ".config.json";

  // SDCard
  static const uint8_t SD_MISO_PIN = 13;
  static const uint8_t SD_MOSI_PIN = 11;
  static const uint8_t SD_SCLK_PIN = 12;

  // Joystick
  static const adc_channel_t ADC_JOYSTICK_X = ADC_CHANNEL_4;
  static const adc_channel_t ADC_JOYSTICK_Y = ADC_CHANNEL_5;
  static const uint8_t JOYSTICK_FIRE_PIN = 18;
  static const uint8_t JOYSTICK_FIRE2_PIN = 17;

  // BLEKB
  static constexpr const char *SERVICE_UUID =
      "695ba701-a48c-43f6-9028-3c885771f19f";
  static constexpr const char *CHARACTERISTIC_UUID =
      "3b05e9bf-086f-4b56-9c37-7b7eeb30b28b";
};

#elif defined(BOARD_T_DISPLAY_S3)

struct Config {
  // --- constants to be defined for each board ---

  // delay until next display refresh
  static const uint8_t REFRESHDELAY = 13;

  // "heuristic performance factor"
  static constexpr double HEURISTIC_PERFORMANCE_FACTOR = 1.0;

  // --- driver specific constants ---

  // power
  static const adc_channel_t BAT_ADC = ADC_CHANNEL_3; // GPIO4

  // filesystem
  static constexpr const char *PATH = "";
  static constexpr const char *CONFIGFILE = ".config.json";

  // display driver
  static const uint16_t LCDWIDTH = 536;
  static const uint16_t LCDHEIGHT = 240;

  // BLEKB
  static constexpr const char *SERVICE_UUID =
      "695ba701-a48c-43f6-9028-3c885771f19f";
  static constexpr const char *CHARACTERISTIC_UUID =
      "3b05e9bf-086f-4b56-9c37-7b7eeb30b28b";
};

#elif defined(BOARD_WAVESHARE)

#define HAS_DEFAULT_VOLUME

struct Config {
  // --- constants to be defined for each board ---

  // delay until next display refresh
  static const uint8_t REFRESHDELAY = 11;

  // "heuristic performance factor"
  static constexpr double HEURISTIC_PERFORMANCE_FACTOR = 0.7;

  // --- driver specific constants ---

  // power
  static const gpio_num_t PWR_KEY_INPUT_PIN = GPIO_NUM_6;
  static const gpio_num_t PWR_CONTROL_PIN = GPIO_NUM_7;
  static const adc_channel_t BAT_ADC = ADC_CHANNEL_7; // GPIO8

  // filesystem
  static constexpr const char *PATH = "";
  static constexpr const char *CONFIGFILE = ".config.json";

  // display driver
  static const uint16_t LCDWIDTH = 320;
  static const uint16_t LCDHEIGHT = 240;

  // ST7789VSerial
  static const int8_t MISO = -1;
  static const uint8_t MOSI = 45;
  static const uint8_t SCLK = 40;
  static const uint8_t LCD_CS = 42;
  static const uint8_t LCD_DC = 41;
  static const uint8_t LCD_RST = 39;
  static const uint8_t LCD_BACKLIGHT_PIN = 5;
  static const uint16_t LCD_BACKLIGHT_FREQUENCY = 20000;
  static const uint8_t LCD_BACKLIGHT_RESOLUTION = 10;
  static const uint16_t LCD_BACKLIGHT_DUTYFACTOR = 500;

  // Sound
  static const uint8_t DEFAULT_VOLUME = 128;
  static const uint8_t I2S_DOUT = 47;
  static const uint8_t I2S_BCLK = 48;
  static const uint8_t I2S_LRC = 38;

  // SDCard
  static const gpio_num_t SD_D3_PIN = GPIO_NUM_21;
  static const uint8_t SD_MISO_PIN = 16;
  static const uint8_t SD_MOSI_PIN = 17;
  static const uint8_t SD_SCLK_PIN = 14;

  // Joystick
  static const adc_channel_t ADC_JOYSTICK_X = ADC_CHANNEL_4;
  static const adc_channel_t ADC_JOYSTICK_Y = ADC_CHANNEL_7;
  static const uint8_t JOYSTICK_FIRE_PIN = 11;
  static const uint8_t JOYSTICK_FIRE2_PIN = 10;

  // BLEKB
  static constexpr const char *SERVICE_UUID =
      "695ba701-a48c-43f6-9028-3c885771f19f";
  static constexpr const char *CHARACTERISTIC_UUID =
      "3b05e9bf-086f-4b56-9c37-7b7eeb30b28b";
};

#else

#error "no valid board defined"

#endif

#else

#error "no valid platform defined"

#endif

#endif // CONFIG_H
