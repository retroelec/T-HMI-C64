/*
 Copyright (C) 2024 retroelec <retroelec42@gmail.com>

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
#ifndef CONFIGSOUND_H
#define CONFIGSOUND_H

#include "Config.h"
#include "SoundDriver.h"
#if defined(USE_I2SSOUND)
#include "I2SSound.h"
#elif defined(USE_NOSOUND)
#include "NoSound.h"
#endif

struct ConfigSound {
  SoundDriver *soundDriver;
#if defined(USE_I2SSOUND)
  ConfigSound() { soundDriver = new I2SSound(); }
#elif defined(USE_NOSOUND)
  ConfigSound() { soundDriver = new NoSound(); }
#endif
};

#endif // CONFIGSOUND_H
