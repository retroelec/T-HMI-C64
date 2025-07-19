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
#ifndef SOUNDDRIVER_H
#define SOUNDDRIVER_H

#include <cstdint>

/**
 * @brief Interface for audio output drivers.
 *
 * Provides a common interface for initializing audio output
 * and submitting audio sample data to the hardware.
 *
 * Implementations of this interface must ensure reliable
 * and synchronized audio playback.
 */
class SoundDriver {
public:
  /**
   * @brief Initializes the audio driver and prepares the audio output system.
   *
   * Is called before calling playAudio(). Sets up required hardware
   * and internal resources for audio playback.
   */
  virtual void init() = 0;

  /**
   * @brief Plays a block of raw 16-bit audio samples.
   *
   * This method is responsible for submitting the provided audio sample data
   * to the underlying audio output system (e.g., DAC, I2S interface, or other).
   *
   * @param samples Pointer to an array of signed 16-bit PCM audio samples.
   * @param size Size of the data in bytes (not the number of samples).
   *
   * Implementations of this method **must** ensure that the provided audio data
   * is fully copied into an internal buffer or transmitted to the audio
   * hardware before the function returns.
   *
   * The function **must block** execution until *all* provided data has been
   * successfully transferred into the internal or hardware-managed buffer.
   *
   * After the function returns, the memory pointed to by `samples` may be
   * reused or overwritten by the caller without affecting audio playback.
   *
   * @note It is the responsibility of the implementation to handle any
   * necessary buffering, synchronization, or hardware interfacing to achieve
   * reliable audio playback.
   */
  virtual void playAudio(int16_t *samples, size_t size) = 0;

  virtual ~SoundDriver() {}
};

#endif // SOUNDDRIVER_H
