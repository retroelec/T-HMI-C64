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
#include "ST7789V.h"
#include <FreeRTOS.h>
#include <driver/gpio.h>
#include <task.h>

static const uint8_t pwr_en = 10;
static const uint8_t bl = 38;

static const uint8_t cs = 6;
static const uint8_t dc = 7;
static const uint8_t wr = 8;

static const uint16_t csval = (1 << 6);
static const uint16_t dcval = (1 << 7);
static const uint16_t wrval = (1 << 8);

static const uint8_t d0 = 48;
static const uint8_t d1 = 47;
static const uint8_t d2 = 39;
static const uint8_t d3 = 40;
static const uint8_t d4 = 41;
static const uint8_t d5 = 42;
static const uint8_t d6 = 45;
static const uint8_t d7 = 46;

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

uint16_t *ST7789V::framecolormem;

void fill_lu_pinbitmask() {
  for (int c = 0; c <= 255; c++) {
    lu_pinbitmask[c] = 0;
    if (c & 1) {
      lu_pinbitmask[c] |= (1 << (d0 - 32));
    }
    if (c & 2) {
      lu_pinbitmask[c] |= (1 << (d1 - 32));
    }
    if (c & 4) {
      lu_pinbitmask[c] |= (1 << (d2 - 32));
    }
    if (c & 8) {
      lu_pinbitmask[c] |= (1 << (d3 - 32));
    }
    if (c & 16) {
      lu_pinbitmask[c] |= (1 << (d4 - 32));
    }
    if (c & 32) {
      lu_pinbitmask[c] |= (1 << (d5 - 32));
    }
    if (c & 64) {
      lu_pinbitmask[c] |= (1 << (d6 - 32));
    }
    if (c & 128) {
      lu_pinbitmask[c] |= (1 << (d7 - 32));
    }
  }
}

esp_err_t config_lcd() {
  gpio_config_t io_conf;
  io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pin_bit_mask =
      (1ULL << d0) | (1ULL << d1) | (1ULL << d2) | (1ULL << d3) | (1ULL << d4) |
      (1ULL << d5) | (1ULL << d6) | (1ULL << d7) | (1ULL << cs) | (1ULL << dc) |
      (1ULL << wr) | (1ULL << bl) | (1ULL << pwr_en);
  return gpio_config(&io_conf);
}

void ST7789V::writeCmd(uint8_t cmd) {
  GPIO.out_w1tc = dcval;
  GPIO.out1_w1tc.val = lu_pinbitmask[255];
  GPIO.out_w1tc = wrval;
  GPIO.out1_w1ts.val = lu_pinbitmask[cmd];
  GPIO.out_w1ts = wrval;
}

void ST7789V::writeData(uint8_t data) {
  GPIO.out_w1ts = dcval;
  GPIO.out1_w1tc.val = lu_pinbitmask[255];
  GPIO.out_w1tc = wrval;
  GPIO.out1_w1ts.val = lu_pinbitmask[data];
  GPIO.out_w1ts = wrval;
}

bool ST7789V::init() {
  ST7789V::framecolormem = (uint16_t *)calloc(320 * 20, sizeof(uint16_t));
  if (framecolormem == nullptr) {
    return false;
  }

  GPIO.out_w1ts = (1 << pwr_en);

  fill_lu_pinbitmask();
  esp_err_t err = config_lcd();
  if (err != ESP_OK) {
    return false;
  }

  GPIO.out_w1tc = csval;

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

  GPIO.out_w1ts = csval;

  GPIO.out1_w1ts.val = (1ULL << (bl - 32)); // backlight
  return true;
}

void ST7789V::copyData(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h,
                       uint16_t *data) {
  uint16_t x1 = x0 + w - 1;
  uint16_t y1 = y0 + h - 1;
  GPIO.out_w1tc = csval;
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
  GPIO.out_w1ts = dcval;
  uint32_t clearMask = lu_pinbitmask[255];
  for (uint32_t i = 0; i < w * h; i++) {
    uint16_t actdata = *data++;
    // writeData(actdata >> 8);
    GPIO.out1_w1tc.val = clearMask;
    GPIO.out_w1tc = wrval;
    GPIO.out1_w1ts.val = lu_pinbitmask[actdata >> 8];
    GPIO.out_w1ts = wrval;
    // writeData(actdata & 0xff);
    GPIO.out1_w1tc.val = clearMask;
    GPIO.out_w1tc = wrval;
    GPIO.out1_w1ts.val = lu_pinbitmask[(uint8_t)actdata];
    GPIO.out_w1ts = wrval;
  }
  writeCmd(nop);
  GPIO.out_w1ts = csval;
}

void ST7789V::drawFrame(uint16_t frameColor) {
  uint16_t cnt = 20 * 320;
  uint16_t *frameptr = ST7789V::framecolormem;
  while (cnt--) {
    *frameptr = frameColor;
    frameptr++;
  }
  ST7789V::copyData(0, 0, 320, 20, ST7789V::framecolormem);
  ST7789V::copyData(0, 220, 320, 20, ST7789V::framecolormem);
}

void ST7789V::drawBitmap(uint16_t *bitmap) {
  ST7789V::copyData(0, 20, 320, 200, bitmap);
}
