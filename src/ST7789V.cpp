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
#include "Config.h"
#ifdef USE_ST7789V
#include "HardwareInitializationException.h"
#include "ST7789V.h"
#include <FreeRTOS.h>
#include <driver/gpio.h>
#include <soc/gpio_struct.h>
#include <task.h>

static const uint16_t CSVAL = (1 << Config::CS);
static const uint16_t DCVAL = (1 << Config::DC);
static const uint16_t WRVAL = (1 << Config::WR);

static const uint8_t nop = 0x00;
static const uint8_t slpin = 0x10;
static const uint8_t slpout = 0x11;
static const uint8_t noron = 0x13;
static const uint8_t dispoff = 0x28;
static const uint8_t dispon = 0x29;
static const uint8_t caset = 0x2a;
static const uint8_t raset = 0x2b;
static const uint8_t ramwr = 0x2c;
static const uint8_t madctl = 0x36;
static const uint8_t colmod = 0x3a;
static const uint8_t ramctrl = 0xb0;

static uint32_t lu_pinbitmask[256];

void fill_lu_pinbitmask() {
  for (int c = 0; c <= 255; c++) {
    lu_pinbitmask[c] = 0;
    if (c & 1) {
      lu_pinbitmask[c] |= (1 << (Config::D0 - 32));
    }
    if (c & 2) {
      lu_pinbitmask[c] |= (1 << (Config::D1 - 32));
    }
    if (c & 4) {
      lu_pinbitmask[c] |= (1 << (Config::D2 - 32));
    }
    if (c & 8) {
      lu_pinbitmask[c] |= (1 << (Config::D3 - 32));
    }
    if (c & 16) {
      lu_pinbitmask[c] |= (1 << (Config::D4 - 32));
    }
    if (c & 32) {
      lu_pinbitmask[c] |= (1 << (Config::D5 - 32));
    }
    if (c & 64) {
      lu_pinbitmask[c] |= (1 << (Config::D6 - 32));
    }
    if (c & 128) {
      lu_pinbitmask[c] |= (1 << (Config::D7 - 32));
    }
  }
}

esp_err_t config_lcd() {
  gpio_config_t io_conf;
  io_conf.intr_type = (gpio_int_type_t)GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pin_bit_mask =
      (1ULL << Config::D0) | (1ULL << Config::D1) | (1ULL << Config::D2) |
      (1ULL << Config::D3) | (1ULL << Config::D4) | (1ULL << Config::D5) |
      (1ULL << Config::D6) | (1ULL << Config::D7) | (1ULL << Config::CS) |
      (1ULL << Config::DC) | (1ULL << Config::WR) | (1ULL << Config::BL);
  return gpio_config(&io_conf);
}

void ST7789V::writeCmd(uint8_t cmd) {
  GPIO.out_w1tc = DCVAL;
  GPIO.out1_w1tc.val = lu_pinbitmask[255];
  GPIO.out_w1tc = WRVAL;
  GPIO.out1_w1ts.val = lu_pinbitmask[cmd];
  GPIO.out_w1ts = WRVAL;
}

void ST7789V::writeData(uint8_t data) {
  GPIO.out_w1ts = DCVAL;
  GPIO.out1_w1tc.val = lu_pinbitmask[255];
  GPIO.out_w1tc = WRVAL;
  GPIO.out1_w1ts.val = lu_pinbitmask[data];
  GPIO.out_w1ts = WRVAL;
}

void ST7789V::init() {
  fill_lu_pinbitmask();
  esp_err_t err = config_lcd();
  if (err != ESP_OK) {
    throw HardwareInitializationException(
        std::string("init. of ST7789V failed: ") + esp_err_to_name(err));
  }

  GPIO.out_w1tc = CSVAL;

  writeCmd(dispoff);
  vTaskDelay(100);

  writeCmd(slpout);
  writeCmd(noron);

  writeCmd(madctl);
  writeData(160);

  writeCmd(ramctrl);
  writeData(0);
  writeData(0);
  writeCmd(colmod);
  writeData(0x05);

  writeCmd(dispon); // display on

  GPIO.out_w1ts = CSVAL;

  GPIO.out1_w1ts.val = (1ULL << (Config::BL - 32)); // backlight
}

void ST7789V::copyinit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h) {
  uint16_t x1 = x0 + w - 1;
  uint16_t y1 = y0 + h - 1;
  GPIO.out_w1tc = CSVAL;
  writeCmd(caset);
  writeData(x0 >> 8);
  writeData(x0 & 0xff);
  writeData(x1 >> 8);
  writeData(x1 & 0xff);
  writeCmd(raset);
  writeData(y0 >> 8);
  writeData(y0 & 0xff);
  writeData(y1 >> 8);
  writeData(y1 & 0xff);
  writeCmd(ramwr);
  GPIO.out_w1ts = DCVAL;
}

void ST7789V::copycopy(uint16_t data, uint32_t clearMask) {
  // writeData(data >> 8);
  GPIO.out1_w1tc.val = clearMask;
  GPIO.out_w1tc = WRVAL;
  GPIO.out1_w1ts.val = lu_pinbitmask[data >> 8];
  GPIO.out_w1ts = WRVAL;
  // writeData(data & 0xff);
  GPIO.out1_w1tc.val = clearMask;
  GPIO.out_w1tc = WRVAL;
  GPIO.out1_w1ts.val = lu_pinbitmask[(uint8_t)data];
  GPIO.out_w1ts = WRVAL;
}

void ST7789V::copyend() {
  writeCmd(nop);
  GPIO.out_w1ts = CSVAL;
}

void ST7789V::copyColor(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h,
                        uint16_t data) {
  copyinit(x0, y0, w, h);
  uint32_t clearMask = lu_pinbitmask[255];
  for (uint32_t i = 0; i < w * h; i++) {
    copycopy(data, clearMask);
  }
  copyend();
}

void ST7789V::copyData(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h,
                       uint16_t *data) {
  copyinit(x0, y0, w, h);
  uint32_t clearMask = lu_pinbitmask[255];
  for (uint32_t i = 0; i < w * h; i++) {
    copycopy(*data++, clearMask);
  }
  copyend();
}

void ST7789V::drawFrame(uint16_t frameColor) {
  if (BORDERHEIGHT > 0) {
    ST7789V::copyColor(BORDERWIDTH, 0, 320, BORDERHEIGHT, frameColor);
    ST7789V::copyColor(BORDERWIDTH, 200 + BORDERHEIGHT, 320, BORDERHEIGHT,
                       frameColor);
  }
  if (BORDERWIDTH > 0) {
    ST7789V::copyColor(0, 0, BORDERWIDTH, Config::LCDHEIGHT, frameColor);
    ST7789V::copyColor(BORDERWIDTH + 320, 0, BORDERWIDTH, Config::LCDHEIGHT,
                       frameColor);
  }
}

void ST7789V::drawBitmap(uint16_t *bitmap) {
  ST7789V::copyData(BORDERWIDTH, BORDERHEIGHT, 320, 200, bitmap);
}

const uint16_t *ST7789V::getC64Colors() const { return c64Colors; }
#endif
