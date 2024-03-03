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
#include "BLEKB.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

static const uint8_t NUMOFCYCLES_KEYPRESSEDDOWN = 4;

uint8_t *buffer;
uint8_t bufidxprod;
uint8_t bufidxcons;
bool deviceConnected;
uint8_t shiftctrlcode;
uint8_t kbcode1;
uint8_t kbcode2;
uint8_t keypresseddowncnt;

class BLEKBServerCallback : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) { deviceConnected = true; };
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    pServer->getAdvertising()->start();
  }
};

class BLEKBCharacteristicCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() >= 4) { // data
      for (uint8_t i = 0; i < value.length(); i++) {
        buffer[i] = (uint8_t)value[i];
        vTaskDelay(1);
      }
      bufidxcons = 0;
      bufidxprod = value.length();
    } else { // keyboard codes
      // BLE client sends 3 codes for each key: dc00, dc01, "shiftctrlcode"
      // shiftctrlcode: bit 0 -> left shift
      //                bit 7 -> external command (cmd, 0, 128)
      for (uint8_t i = 0; i < value.length(); i++) {
        buffer[i] = (uint8_t)value[i];
      }
      shiftctrlcode = buffer[2];
      keypresseddowncnt = 0;
    }
    pCharacteristic->setValue("K");
    pCharacteristic->notify();
  }
};

void BLEKB::init(std::string service_uuid, std::string characteristic_uuid,
                 uint8_t *kbbuffer) {
  // init buffer
  keypresseddowncnt = NUMOFCYCLES_KEYPRESSEDDOWN;
  bufidxprod = 0;
  bufidxcons = 0;
  buffer = kbbuffer;
  kbcode1 = 0xff;
  kbcode2 = 0xff;

  // init BLE
  deviceConnected = false;
  BLEDevice::init("THMIC64");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BLEKBServerCallback());
  BLEService *pService = pServer->createService(service_uuid);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      characteristic_uuid,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->setCallbacks(new BLEKBCharacteristicCallback());
  pCharacteristic->setValue("THMIC64");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

uint8_t BLEKB::getKBCode() {
  // "external" command?
  if ((shiftctrlcode & 128) != 0) {
    shiftctrlcode = 0;
    kbcode1 = 0xff;
    keypresseddowncnt = NUMOFCYCLES_KEYPRESSEDDOWN;
    return buffer[0];
  }
  // key is "pressed down" at least for 50 ms
  if (keypresseddowncnt < NUMOFCYCLES_KEYPRESSEDDOWN) {
    kbcode2 = buffer[0];
    kbcode1 = buffer[1];
    keypresseddowncnt++;
  } else {
    kbcode1 = 0xff;
    kbcode2 = 0xff;
  }
  return 0;
}

uint8_t BLEKB::decode(uint8_t dc00) {
  if (dc00 == 0) {
    return kbcode1;
  } else {
    if (((~dc00 & 2) != 0) && ((shiftctrlcode & 1) != 0)) {
      // left shift pressed
      if (kbcode2 == 0xfd) {
        // handle scan of key codes in the same "row"
        return kbcode1 & 0x7f;
      } else {
        return 0x7f;
      }
    }
    if (dc00 == kbcode2) {
      return kbcode1;
    } else {
      return 0xff;
    }
  }
}

bool BLEKB::getData(uint8_t *data) {
  if (bufidxprod != bufidxcons) {
    *data = buffer[bufidxcons++];
    return true;
  }
  return false;
}
