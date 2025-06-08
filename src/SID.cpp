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
#include <cmath>

static const float attackLUT[16] = {
    0.002f, 0.008f, 0.016f, 0.024f, 0.038f, 0.056f, 0.068f, 0.080f,
    0.1f,   0.25f,  0.5f,   0.8f,   1.0f,   3.0f,   5.0f,   8.0f};

static const float releaseDecayLUT[16] = {
    0.008f, 0.024f, 0.048f, 0.072f, 0.114f, 0.168f, 0.204f, 0.24f,
    0.3f,   0.75f,  1.5f,   2.4f,   3.0f,   9.0f,   15.0f,  24.0f};

SIDVoice::SIDVoice() { init(); }

void SIDVoice::init() {
  adsrState = IDLE;
  control = 0;
  lfsr = 0x7FFFF8;
  phase = 1.0f;
  phaseIncrement = 0.0f;
  envelope = 0.0f;
  syncNextVoice = false;
  ringmod = false;
  v3env = 0.0f;
  pulseWidth = 0.0f;
  sustainVolume = 0.0f;
  attackAdd = 0.0f;
  decayAdd = 0.0f;
  releaseAdd = 0.0f;
  noiseValue = 0.0f;
}

bool SIDVoice::isActive() { return adsrState != IDLE; }

void SIDVoice::updVarFrequency(uint16_t freq) {
  phaseIncrement =
      (float)(freq)*985248.0f / 16777216.0f / Config::AUDIO_SAMPLE_RATE;
}

void SIDVoice::updVarPulseWidth(uint16_t pw) {
  pulseWidth = (float)(pw & 0x0fff) / 4095.0;
}

void SIDVoice::updVarEnvelopeAD(uint8_t val) {
  attackAdd = 1.0f / (attackLUT[(val >> 4) & 0x0f] * Config::AUDIO_SAMPLE_RATE);
  decayAdd = (1.0f - sustainVolume) /
             (releaseDecayLUT[val & 0x0f] * Config::AUDIO_SAMPLE_RATE);
}

void SIDVoice::updVarEnvelopeSR(uint8_t val) {
  sustainVolume = (float)(val & 0x0f) / 15.0;
  float time = releaseDecayLUT[val & 0x0f];
  // special case: sustainVolume may be 0 -> enforce some reasonable value for
  // releaseAdd
  releaseAdd = sustainVolume > 0.0f
                   ? sustainVolume / (time * Config::AUDIO_SAMPLE_RATE)
                   : 1.0f / (time * Config::AUDIO_SAMPLE_RATE);
}

void SIDVoice::updVarControl(uint8_t val) {
  uint8_t oldControl = control;
  control = val;
  bool oldGate = oldControl & 0x01;
  bool newGate = control & 0x01;
  // bit 0 (gate bit)
  if (!oldGate && newGate) {
    adsrState = ATTACK;
    envelope = 0.0f;
  } else if (oldGate && !newGate) {
    adsrState = RELEASE;
  }
  // bit 1 (sync)
  if (voice != 0) {
    if (control & 0x02) {
      prevVoice->syncNextVoice = true;
    } else {
      prevVoice->syncNextVoice = false;
    }
  }
  // bit 2 (ringmod)
  ringmod = control & 0x04;
  // bit 3 (test)
  bool oldTest = oldControl & 0x08;
  bool newTest = control & 0x08;
  if (!oldTest && newTest) {
    phase = 0.0f;
    phaseIncrement = 0.0f;
    lfsr = 0x7FFFF8;
  }
  // bit 4-7 (waveform)
  uint8_t oldWave = oldControl & 0xf0;
  uint8_t newWave = control & 0xf0;
  if (newGate && (oldWave != newWave)) {
    adsrState = ATTACK;
    phase = 0.0f;
  }
}

float SIDVoice::updateEnvelope() {
  switch (adsrState) {
  case ATTACK:
    envelope += attackAdd;
    if (envelope >= 1.0f) {
      envelope = 1.0f;
      adsrState = DECAY;
    }
    break;
  case DECAY:
    envelope -= decayAdd;
    if (envelope <= sustainVolume) {
      envelope = sustainVolume;
      adsrState = SUSTAIN;
    }
    break;
  case SUSTAIN:
    break;
  case RELEASE:
    envelope -= releaseAdd;
    if (envelope <= 0.0f) {
      envelope = 0.0f;
      adsrState = IDLE;
    }
    break;
  case IDLE:
    envelope = 0.0f;
    break;
  }
  if (voice == 2) {
    v3env = envelope;
  }
  return envelope;
}

void SIDVoice::nextLFSR() {
  bool bit22 = (lfsr >> 22) & 1;
  bool bit17 = (lfsr >> 17) & 1;
  lfsr = ((lfsr << 1) | (bit22 ^ bit17));
}

float SIDVoice::getNoiseNormalized() const {
  uint16_t noise12bit = (((lfsr >> 22) & 1) << 11) |
                        (((lfsr >> 20) & 1) << 10) | (((lfsr >> 16) & 1) << 9) |
                        (((lfsr >> 13) & 1) << 8) | (((lfsr >> 11) & 1) << 7) |
                        (((lfsr >> 7) & 1) << 6) | (((lfsr >> 6) & 1) << 5) |
                        (((lfsr >> 3) & 1) << 4) | (((lfsr >> 1) & 1) << 3) |
                        (((lfsr >> 0) & 1) << 2) | (((lfsr >> 18) & 1) << 1) |
                        (((lfsr >> 14) & 1) << 0);
  return ((float)noise12bit / 2047.5f) - 1.0f;
}

float SIDVoice::generateSample() {
  // 0 <= phase < 1
  // -1 <= sample < 1
  phase += phaseIncrement;
  if (phase >= 1.0f) {
    phase = 0.0f;
    if (syncNextVoice) { // syncNextVoice is never true for voice 3 (index 2)
      nextVoice->phase = 0.0f;
    }
  }
  bool active = isActive();
  float sample = 0.0f;
  uint8_t wavecnt = 0;
  if (active) {
    // triangle
    if (control & 0x10) {
      float triangle = 4.0f * fabsf(phase - 0.5f) - 1.0f;
      if (ringmod && prevVoice->phase >= 0.5f) {
        triangle = -triangle;
      }
      sample += triangle;
      wavecnt++;
    }
    // saw
    if (control & 0x20) {
      sample += 2.0f * phase - 1.0f;
      wavecnt++;
    }
    // pulse
    if (control & 0x40) {
      sample += (phase < pulseWidth) ? 1.0f : -1.0f;
      wavecnt++;
    }
  }
  // noise
  if (control & 0x80) {
    if (phase < phaseIncrement) {
      nextLFSR();
      if (active) {
        noiseValue = getNoiseNormalized();
      }
    }
    if (active) {
      sample += noiseValue;
      wavecnt++;
    }
  }
  if (wavecnt > 1) {
    sample /= (float)wavecnt;
  }
  return sample;
}

void SID::init() {
  emuVolume = 30000.0f;
  bufferfilled = false;
  for (uint8_t i = 0; i < 0x20; i++) {
    sidreg[i] = 0;
  }
  // use fillBuffer method to clear samples array
  buffer0filled = false;
  for (int i = 0; i < 3; i++) {
    sidVoice[i].init();
  }
  sidVoice[0].voice = 0;
  sidVoice[1].voice = 1;
  sidVoice[2].voice = 2;
  sidVoice[0].nextVoice = &sidVoice[1];
  sidVoice[1].nextVoice = &sidVoice[2];
  sidVoice[2].nextVoice = nullptr;
  sidVoice[0].prevVoice = &sidVoice[2];
  sidVoice[1].prevVoice = &sidVoice[0];
  sidVoice[2].prevVoice = &sidVoice[1];
}

SID::SID() {
  configSound.soundDriver->init();
  init();
}

bool SID::isVoiceActive() {
  for (int i = 0; i < 3; i++) {
    if (sidVoice[i].isActive()) {
      return true;
    }
  }
  return false;
}

int16_t SID::generateSample() {
  float sample = 0.0f;
  uint8_t cnt = 0;
  for (int i = 0; i < 3; i++) {
    if (sidVoice[i].isActive()) {
      cnt++;
    }
    float env = sidVoice[i].updateEnvelope();
    sample += env * sidVoice[i].generateSample();
  }
  if (cnt == 0) {
    return 0;
  }
  sample /= cnt;
  return static_cast<int16_t>(sample * c64Volume * emuVolume);
}

void SID::fillBuffer() {
#ifndef USE_NOSOUND
  if (!isVoiceActive()) {
    if (buffer0filled) {
      return;
    }
    for (uint16_t i = 0; i < NUMSAMPLESPERFRAME; i++) {
      samples[i] = 0;
    }
    buffer0filled = true;
  } else {
    buffer0filled = false;
  }
  for (uint16_t i = 0; i < NUMSAMPLESPERFRAME; i++) {
    samples[i] = generateSample();
    v3envs[i] = sidVoice[2].v3env;
  }
  bufferfilled = true;
#endif
}

void SID::playAudio() {
  if (!bufferfilled) {
    return;
  }
  configSound.soundDriver->playAudio(samples,
                                     NUMSAMPLESPERFRAME * sizeof(int16_t));
  bufferfilled = false;
}
