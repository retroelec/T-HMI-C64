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
#ifndef SID_H
#define SID_H

#include "ConfigSound.h"
#include <cstdint>

class SIDVoice {
private:
  enum ADSRState { ATTACK, DECAY, SUSTAIN, RELEASE, IDLE };
  enum Waveform {
    WAVE_TRIANGLE,
    WAVE_SAWTOOTH,
    WAVE_PULSE,
    WAVE_NOISE,
    NOWAVE
  };

  // extract from SID registers
  float freqHz;
  float pulseWidth;
  float attack;
  float decay;
  float sustainVolume;
  float release;
  float attackCoeff;
  float decayCoeff;
  float releaseCoeff;

  ADSRState adsrState = IDLE;
  float envelope = 0.0f;
  Waveform waveform;
  float noiseValue;
  float phase = 0.0f;

  float updateEnvelope();

public:
  SIDVoice();
  void init();
  bool isActive();
  void start(uint8_t *voicereg, uint8_t val);
  void stop();
  float generateSample(float masterVolume);
};

class SID {
private:
  static const uint16_t NUMSAMPLESPERFRAME = Config::AUDIO_SAMPLE_RATE / 50;
  int16_t samples[NUMSAMPLESPERFRAME];

  ConfigSound configSound;
  bool bufferfilled;

  SIDVoice sidVoice[3];

  int16_t generateSample();
  bool isVoiceActive();

public:
  float masterVolume;
  uint8_t sidreg[0x20];

  SID();
  void init();
  void startSound(uint8_t voice, uint8_t val);
  void stopSound(uint8_t voice);
  void fillBuffer();
  void playAudio();
};
#endif // SID_H
