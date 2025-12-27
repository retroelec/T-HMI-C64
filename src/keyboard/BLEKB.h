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

#include "../Config.h"
#ifdef USE_BLE_KEYBOARD
#include "KeyboardDriver.h"
#include <BLEServer.h>
#include <atomic>
#include <cstdint>

class BLEKB : public KeyboardDriver {
private:
  BLECharacteristic *pCharacteristic;
  std::atomic<uint8_t> sentdc01;
  std::atomic<uint8_t> sentdc00;
  std::atomic<bool> detectreleasekey;

public:
  uint8_t *buffer;
  std::atomic<bool> deviceConnected;
  std::atomic<uint8_t> shiftctrlcode;
  std::atomic<uint8_t> keypresseddowncnt;
  std::atomic<uint8_t> virtjoystickvalue;
  std::atomic<bool> keypresseddown;

  BLEKB();
  void init() override;
  uint8_t *getExtCmdData() override;
  void sendExtCmdNotification(uint8_t *data, size_t size) override;
  void scanKeyboard() override;
  uint8_t getKBCodeDC01() override;
  uint8_t getKBCodeDC00() override;
  uint8_t getShiftctrlcode() override;
  uint8_t getKBJoyValue() override;
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
#endif

#endif // BLEKB_H
