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
#ifndef BLEKB_H
#define BLEKB_H

#include "KeyboardDriver.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <cstdint>
#include <string>

class BLEKB : public KeyboardDriver {
private:
  BLECharacteristic *pCharacteristic;
  uint8_t sentdc01;
  uint8_t sentdc00;
  bool detectreleasekey;

public:
  portMUX_TYPE blekbmutex = portMUX_INITIALIZER_UNLOCKED;
  bool deviceConnected;
  uint8_t *buffer;
  uint8_t shiftctrlcode;
  uint8_t keypresseddowncnt;
  uint8_t virtjoystickvalue;
  bool keypresseddown;

  BLEKB();
  void init() override;
  uint8_t *getExtCmdBuffer() override;
  void sendExtCmdNotification(uint8_t *data, size_t size) override;
  void scanKeyboard() override;
  uint8_t getDC01(uint8_t dc00, bool xchgports) override;
  uint8_t getKBJoyValue(bool port2) override;
  void setKBcodes(uint8_t sentdc01, uint8_t sentdc00) override;
  void setDetectReleasekey(bool detectreleasekey) override;
};

class BLEKBServerCallback : public BLEServerCallbacks {
private:
  BLEKB &blekb;

public:
  BLEKBServerCallback(BLEKB &blekb);
  void onConnect(BLEServer *pServer);
  void onDisconnect(BLEServer *pServer);
};

class BLEKBCharacteristicCallback : public BLECharacteristicCallbacks {
private:
  BLEKB &blekb;

public:
  BLEKBCharacteristicCallback(BLEKB &blekb);
  void onWrite(BLECharacteristic *pCharacteristic);
};

#endif // BLEKB_H
