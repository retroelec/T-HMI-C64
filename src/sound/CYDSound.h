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
#include <cstring>
#include <driver/dac_continuous.h>
#include <freertos/FreeRTOS.h>

class CYDSound : public SoundDriver {
private:
  static constexpr size_t FRAMESIZE = AUDIO_SAMPLE_RATE / 50;

  dac_continuous_handle_t dac_handle = nullptr;
  uint8_t sampleBuf[FRAMESIZE];
  size_t lastSize;

  static bool onConvertDone(dac_continuous_handle_t handle,
                            const dac_event_data_t *event, void *user_data) {
    CYDSound *self = (CYDSound *)user_data;
    if (self->lastSize > 0) {
      size_t copy =
          self->lastSize < event->buf_size ? self->lastSize : event->buf_size;
      memcpy(event->buf, self->sampleBuf, copy);
    }
    return false;
  }

public:
  void init() override {
    memset(sampleBuf, 128, FRAMESIZE);
    lastSize = 0;

    dac_continuous_config_t cont_cfg = {
        .chan_mask = DAC_CHANNEL_MASK_CH1,
        .desc_num = 8,
        .buf_size = 512,
        .freq_hz = AUDIO_SAMPLE_RATE,
        .offset = 0,
        .clk_src = DAC_DIGI_CLK_SRC_DEFAULT,
        .chan_mode = DAC_CHANNEL_MODE_SIMUL,
    };
    ESP_ERROR_CHECK(dac_continuous_new_channels(&cont_cfg, &dac_handle));

    dac_event_callbacks_t cbs = {};
    cbs.on_convert_done = onConvertDone;
    ESP_ERROR_CHECK(
        dac_continuous_register_event_callback(dac_handle, &cbs, this));

    ESP_ERROR_CHECK(dac_continuous_enable(dac_handle));
    ESP_ERROR_CHECK(dac_continuous_start_async_writing(dac_handle));
  }

  void playAudio(int16_t *samples, size_t size) override {
    size_t numSamples = size / sizeof(int16_t);
    size_t n = numSamples < FRAMESIZE ? numSamples : FRAMESIZE;
    for (size_t i = 0; i < n; i++) {
      sampleBuf[i] = (samples[i] + 32768) >> 8;
    }
    lastSize = n;
  }

  ~CYDSound() {
    if (dac_handle) {
      dac_continuous_disable(dac_handle);
      dac_continuous_del_channels(dac_handle);
    }
  }
};
#endif

#endif // CYDSOUND_H