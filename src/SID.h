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

  float phase;
  float phaseIncrement;
  float pulseWidth;
  float sustainVolume;
  float attackAdd;
  float decayAdd;
  float releaseAdd;
  ADSRState adsrState;
  float noiseValue;
  uint32_t lfsr;
  bool syncNextVoice;
  bool ringmod;

  void nextLFSR();
  float getNoiseNormalized() const;

public:
  uint8_t control;
  uint8_t voice;
  float sample;
  float envelope;
  SIDVoice *nextVoice;
  SIDVoice *prevVoice;

  SIDVoice();
  void init();
  bool isActive();
  void updVarFrequency(uint16_t freq);
  void updVarPulseWidth(uint16_t pw);
  void updVarEnvelopeAD(uint8_t val);
  void updVarEnvelopeSR(uint8_t val);
  void updVarControl(uint8_t val);
  float updateEnvelope();
  float generateSample();
};

class SID {
private:
  static const uint16_t NUMSAMPLESPERFRAME = Config::AUDIO_SAMPLE_RATE / 50;
  int16_t samples[NUMSAMPLESPERFRAME];
  ConfigSound sound;
  uint16_t actSampleIdx;
  bool voice2silent;

  int16_t generateSample();

public:
  SIDVoice sidVoice[3];
  float c64Volume;
  float emuVolume;
  uint8_t sidreg[0x20];

  SID();
  void init();
  void startSound(uint8_t voice, uint8_t val);
  void stopSound(uint8_t voice);
  void fillBuffer(uint16_t rasterline);
  void playAudio();
};
#endif // SID_H
