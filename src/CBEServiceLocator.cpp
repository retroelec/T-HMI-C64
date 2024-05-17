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
#include "CBEServiceLocator.h"

CPUC64 *CBEServiceLocator::cpuC64;
BLEKB *CBEServiceLocator::blekb;
ExternalCmds *CBEServiceLocator::externalCmds;

CPUC64 *CBEServiceLocator::getCPUC64() { return CBEServiceLocator::cpuC64; }

void CBEServiceLocator::setCPUC64(CPUC64 *cpuC64) {
  CBEServiceLocator::cpuC64 = cpuC64;
}

ExternalCmds *CBEServiceLocator::getExternalCmds() {
  return CBEServiceLocator::externalCmds;
}

void CBEServiceLocator::setExternalCmds(ExternalCmds *externalCmds) {
  CBEServiceLocator::externalCmds = externalCmds;
}

BLEKB *CBEServiceLocator::getBLEKB() { return CBEServiceLocator::blekb; }

void CBEServiceLocator::setBLEKB(BLEKB *blekb) {
  CBEServiceLocator::blekb = blekb;
}
