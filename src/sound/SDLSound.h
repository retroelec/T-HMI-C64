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
#ifndef SDLSOUND_H
#define SDLSOUND_H

#include "../Config.h"
#ifdef USE_SDLSOUND
#include "SoundDriver.h"
#include <SDL2/SDL.h>
#include <mutex>
#include <queue>
#include <vector>

class SDLSound : public SoundDriver {
public:
private:
  SDL_AudioDeviceID audioDevice = 0;
  std::deque<int16_t> audioBuffer;
  std::mutex bufferMutex;
  bool initialized;
  bool quit;

  static void audioCallbackStatic(void *userdata, Uint8 *stream, int len) {
    static_cast<SDLSound *>(userdata)->audioCallback((int16_t *)stream,
                                                     len / sizeof(int16_t));
  }

  void audioCallback(int16_t *stream, int len) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    for (int i = 0; i < len; ++i) {
      if (!audioBuffer.empty()) {
        stream[i] = audioBuffer.front();
        audioBuffer.pop_front();
      } else {
        stream[i] = 0; // silence
      }
    }
  }

public:
  SDLSound() : initialized(false), quit(false) {}

  void init() override {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      throw std::runtime_error("SDL_Init failed: " +
                               std::string(SDL_GetError()));
    }
    SDL_AudioSpec desiredSpec = {};
    desiredSpec.freq = AUDIO_SAMPLE_RATE;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 512;
    desiredSpec.callback = &SDLSound::audioCallbackStatic;
    desiredSpec.userdata = this;
    SDL_AudioSpec obtainedSpec;
    audioDevice =
        SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &obtainedSpec, 0);
    if (audioDevice == 0) {
      throw std::runtime_error("SDL_OpenAudioDevice failed: " +
                               std::string(SDL_GetError()));
    }
    SDL_PauseAudioDevice(audioDevice, 0);
    initialized = true;
  }

  void playAudio(int16_t *samples, size_t size) override {
    if (!initialized) {
      return;
    }
    std::lock_guard<std::mutex> lock(bufferMutex);
    audioBuffer.insert(audioBuffer.end(), samples,
                       samples + (size / sizeof(int16_t)));
  }

  ~SDLSound() override {
    quit = true;
    if (audioDevice) {
      SDL_CloseAudioDevice(audioDevice);
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  }
};
#endif

#endif // SDLSOUND_H
