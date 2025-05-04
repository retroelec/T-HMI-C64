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

#include "SID.h"
#include "jllog.h"
#include <algorithm>
#include <cmath>

static const float attackLUT[16] = {
    0.002f, 0.008f, 0.016f, 0.024f, 0.038f, 0.056f, 0.068f, 0.080f,
    0.1f,   0.25f,  0.5f,   0.8f,   1.0f,   3.0f,   5.0f,   8.0f};

static const float releaseDecayLUT[16] = {
    0.008f, 0.024f, 0.048f, 0.072f, 0.114f, 0.168f, 0.204f, 0.24f,
    0.3f,   0.75f,  1.5f,   2.4f,   3.0f,   9.0f,   15.0f,  24.0f};

SIDVoice::SIDVoice() { init(); }

float SIDVoice::updateEnvelope() {
  switch (adsrState) {
  case ATTACK:
    envelope += (1.0f - envelope) * attackCoeff;
    if (envelope >= 0.99f) {
      envelope = 1.0f;
      adsrState = DECAY;
    }
    break;
  case DECAY:
    envelope += (sustainVolume - envelope) * decayCoeff;
    if (envelope <= sustainVolume + 0.001f) {
      envelope = sustainVolume;
      adsrState = SUSTAIN;
    }
    break;
  case SUSTAIN:
    break;
  case RELEASE:
    envelope += (0.0f - envelope) * releaseCoeff;
    if (envelope <= 0.001f) {
      envelope = 0.0f;
      adsrState = IDLE;
    }
    break;
  case IDLE:
    envelope = 0.0f;
    break;
  }
  return std::clamp(envelope, 0.0f, 1.0f);
}

void SIDVoice::init() { adsrState = IDLE; }

bool SIDVoice::isActive() { return adsrState != IDLE; }

float computeCoefficient(float rateInSeconds) {
  if (rateInSeconds <= 0.0f)
    return 1.0f;
  float samples = rateInSeconds * Config::AUDIO_SAMPLE_RATE;
  return 1.0f - std::exp(-1.0f / samples);
}

void SIDVoice::start(uint8_t *voicereg, uint8_t control) {
  freqHz = (float)(voicereg[0] | (voicereg[1] << 8)) * 985248.0f / 16777216.0f;
  attack = attackLUT[(voicereg[5] >> 4) & 0x0f];
  decay = releaseDecayLUT[voicereg[5] & 0x0f];
  sustainVolume = (float)((voicereg[6] >> 4) & 0x0f) / 15.0;
  release = releaseDecayLUT[voicereg[6] & 0x0f];
  adsrState = ATTACK;
  waveform = NOWAVE;
  if (control & 0x80) {
    waveform = WAVE_NOISE;
  } else if (control & 0x40) {
    waveform = WAVE_PULSE;
    pulseWidth =
        (float)((voicereg[2] | ((voicereg[3] & 0x0f) << 8)) & 0x0fff) / 4095.0;
  } else if (control & 0x20) {
    waveform = WAVE_SAWTOOTH;
  } else if (control & 0x10) {
    waveform = WAVE_TRIANGLE;
  }
  envelope = 0.0f;
  phase = 0.0f;
  attackCoeff = computeCoefficient(attack);
  decayCoeff = computeCoefficient(decay);
  releaseCoeff = computeCoefficient(release);
}

void SIDVoice::stop() { adsrState = RELEASE; }

float SIDVoice::generateSample(float masterVolume) {
  // 0 <= phase < 1
  // -1 <= sample < 1
  float phaseIncrement = freqHz / Config::AUDIO_SAMPLE_RATE;
  phase += phaseIncrement;
  if (phase >= 1.0f) {
    phase -= 1.0f;
  }
  float sample = 0.0f;
  switch (waveform) {
  case NOWAVE:
    return 0.0f;
  case WAVE_TRIANGLE:
    sample = (phase < 0.5f) ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase);
    break;
  case WAVE_SAWTOOTH:
    sample = 2.0f * phase - 1.0f;
    break;
  case WAVE_PULSE:
    sample = (phase < pulseWidth) ? 1.0f : -1.0f;
    break;
  case WAVE_NOISE:
    if (phase < phaseIncrement) {
      noiseValue = ((float)(rand() % 65536) / 32768.0f) - 1.0f;
    }
    sample = noiseValue;
    break;
  }
  float env = updateEnvelope();
  return sample * env * masterVolume;
}

SID::SID() { init(); }

int16_t SID::generateSample() {
  float sample = 0.0f;
  uint8_t cnt = 0;
  for (int i = 0; i < 3; i++) {
    if (sidVoice[i].isActive()) {
      cnt++;
      sample += sidVoice[i].generateSample(masterVolume);
    }
  }
  if (cnt == 0) {
    return 0.0;
  }
  sample /= cnt;
  return static_cast<int16_t>(sample * 30000.0f);
}

bool SID::isVoiceActive() {
  for (int i = 0; i < 3; i++) {
    if (sidVoice[i].isActive()) {
      return true;
    }
  }
  return false;
}

void SID::init() {
  configSound.soundDriver->init();
  bufferfilled = false;
  for (uint8_t i = 0; i < 0x20; i++) {
    sidreg[i] = 0;
  }
  for (int i = 0; i < 3; i++) {
    sidVoice[i].init();
  }
}

void SID::startSound(uint8_t voice, uint8_t val) {
  sidVoice[voice].start(&sidreg[voice * 7], val);
}

void SID::stopSound(uint8_t voice) { sidVoice[voice].stop(); }

void SID::fillBuffer() {
  if (!isVoiceActive()) {
    return;
  }
  for (uint16_t i = 0; i < NUMSAMPLESPERFRAME; i++) {
    samples[i] = generateSample();
  }
  bufferfilled = true;
}

void SID::playAudio() {
  if (!bufferfilled) {
    return;
  }
  configSound.soundDriver->playAudio(samples,
                                     NUMSAMPLESPERFRAME * sizeof(int16_t));
  bufferfilled = false;
}
