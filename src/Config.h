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

#include <driver/adc.h>
#include <stdint.h>
#include <string>

namespace Config {
// SDCard
static const uint8_t PWR_EN_PIN = 10;
static const uint8_t SD_MISO_PIN = 13;
static const uint8_t SD_MOSI_PIN = 11;
static const uint8_t SD_SCLK_PIN = 12;

// BLEKB
static const std::string SERVICE_UUID = "695ba701-a48c-43f6-9028-3c885771f19f";
static const std::string CHARACTERISTIC_UUID =
    "3b05e9bf-086f-4b56-9c37-7b7eeb30b28b";

// Joystick
static const adc2_channel_t ADC_JOYSTICK_X = ADC2_CHANNEL_4;
static const adc2_channel_t ADC_JOYSTICK_Y = ADC2_CHANNEL_5;
static const uint8_t JOYSTICK_FIRE_PIN = 18;
}; // namespace Config

#endif // CONFIG_H
