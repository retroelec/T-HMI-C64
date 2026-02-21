/*
 Copyright (C) 2024-2026 retroelec <retroelec42@gmail.com>

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
#ifndef CYDSOUND_H
#define CYDSOUND_H

#include "../Config.h"
#ifdef USE_CYDSOUND
#include "SoundDriver.h"
#include "driver/dac_oneshot.h"
#include "soc/sens_reg.h"
#include <driver/i2s_std.h>
#include <freertos/FreeRTOS.h>

#ifndef SENS_DAC_DIG_FORCE_M
#define SENS_DAC_DIG_FORCE_M (BIT(2))
#endif
#ifndef SENS_DAC_I2S_EN_M
#define SENS_DAC_I2S_EN_M (BIT(3))
#endif

class CYDSound : public SoundDriver {
private:
  i2s_chan_handle_t tx_channel = nullptr;

public:
  void init() override {
    i2s_chan_config_t chan_cfg =
        I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;
    chan_cfg.dma_desc_num = 16;
    chan_cfg.dma_frame_num = 256;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_channel, NULL));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg =
            {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = I2S_GPIO_UNUSED,
                .ws = I2S_GPIO_UNUSED,
                .dout = (gpio_num_t)Config::I2S_DOUT,
                .din = I2S_GPIO_UNUSED,
            },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_channel, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_channel));

    // DAC hardware routing
    SET_PERI_REG_MASK(SENS_SAR_DAC_CTRL1_REG,
                      SENS_DAC_DIG_FORCE_M | SENS_DAC_I2S_EN_M);
    SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_INV2, 2,
                      SENS_DAC_INV2_S);
  }

  void playAudio(int16_t *samples, size_t size) {
    size_t bw = 0;
    ESP_ERROR_CHECK(
        i2s_channel_write(tx_channel, samples, size, &bw, portMAX_DELAY));
  }

  ~CYDSound() {
    if (tx_channel) {
      i2s_channel_disable(tx_channel);
      i2s_del_channel(tx_channel);
    }
  }
};
#endif

#endif // CYDSOUND_H
