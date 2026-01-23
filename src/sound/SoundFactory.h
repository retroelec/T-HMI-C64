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
#ifndef SOUNDFACTORY_H
#define SOUNDFACTORY_H

#include "../Config.h"
#include "SoundDriver.h"
#if defined(USE_I2SSOUND)
#include "I2SSound.h"
#elif defined(USE_SDLSOUND)
#include "SDLSound.h"
#elif defined(USE_NOSOUND)
#include "NoSound.h"
#else
#error "no valid sound driver defined"
#endif

namespace Sound {
SoundDriver *create() {
#if defined(USE_I2SSOUND)
  return new I2SSound();
#elif defined(USE_SDLSOUND)
  return new SDLSound();
#elif defined(USE_NOSOUND)
  return new NoSound();
#endif
}
} // namespace Sound

#endif // SOUNDFACTORY_H
