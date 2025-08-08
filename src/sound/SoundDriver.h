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

#include <cstddef>
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
   * Must block until all provided data has been safely queued or copied
   * into the internal audio buffer.
   *
   * The function does not need to wait for the data to be fully played
   * on the hardware, but must ensure that the caller can reuse or overwrite
   * the provided memory after the function returns.
   */
  virtual void playAudio(int16_t *samples, size_t size) = 0;

  virtual ~SoundDriver() {}
};

#endif // SOUNDDRIVER_H
