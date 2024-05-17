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
#ifndef CBESERVICELOCATOR_H
#define CBESERVICELOCATOR_H

#include "BLEKB.h"
#include "CPUC64.h"
#include "ExternalCmds.h"

class CBEServiceLocator {
private:
  static CPUC64 *cpuC64;
  static BLEKB *blekb;
  static ExternalCmds *externalCmds;

public:
  static CPUC64 *getCPUC64();
  static void setCPUC64(CPUC64 *cpuc64);
  static ExternalCmds *getExternalCmds();
  static void setExternalCmds(ExternalCmds *externalCmds);
  static BLEKB *getBLEKB();
  static void setBLEKB(BLEKB *blekbInstance);
};
#endif // CBESERVICELOCATOR_H
