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
#include "Config.h"
#include "Joystick.h"
#include <esp_log.h>

static const char *TAG = "BLEKB";

static const uint8_t NUMOFCYCLES_KEYPRESSEDDOWN = 4;

static const uint8_t VIRTUALJOYSTICKLEFT_ACTIVATED = 0x01;
static const uint8_t VIRTUALJOYSTICKLEFT_DEACTIVATED = 0x81;
static const uint8_t VIRTUALJOYSTICKRIGHT_ACTIVATED = 0x02;
static const uint8_t VIRTUALJOYSTICKRIGHT_DEACTIVATED = 0x82;
static const uint8_t VIRTUALJOYSTICKUP_ACTIVATED = 0x04;
static const uint8_t VIRTUALJOYSTICKUP_DEACTIVATED = 0x84;
static const uint8_t VIRTUALJOYSTICKDOWN_ACTIVATED = 0x08;
static const uint8_t VIRTUALJOYSTICKDOWN_DEACTIVATED = 0x88;

BLEKBServerCallback::BLEKBServerCallback(BLEKB &blekb) : blekb(blekb) {}

void BLEKBServerCallback::onConnect(BLEServer *pServer) {
  blekb.deviceConnected = true;
  ESP_LOGI(TAG, "BLE device connected");
};

void BLEKBServerCallback::onDisconnect(BLEServer *pServer) {
  blekb.deviceConnected = false;
  ESP_LOGI(TAG, "BLE device disconnected, try to connect...");
  pServer->getAdvertising()->start();
}

BLEKBCharacteristicCallback::BLEKBCharacteristicCallback(BLEKB &blekb)
    : blekb(blekb) {}

void BLEKBCharacteristicCallback::onWrite(BLECharacteristic *pCharacteristic) {
  if (!blekb.deviceConnected) {
    return;
  }
  std::string value = pCharacteristic->getValue();
  if (value.length() >= 4) { // data
    for (uint8_t i = 0; i < value.length(); i++) {
      blekb.buffer[i] = (uint8_t)value[i];
      vTaskDelay(1);
    }
    blekb.bufidxcons = 0;
    blekb.bufidxprod = value.length();
  } else if (value.length() == 3) { // keyboard codes
    // BLE client sends 3 codes for each key: dc00, dc01, "shiftctrlcode"
    // shiftctrlcode: bit 0 -> left shift
    //                bit 7 -> external command (cmd, 0, 128)
    for (uint8_t i = 0; i < 3; i++) {
      blekb.buffer[i] = (uint8_t)value[i];
    }
    blekb.shiftctrlcode = blekb.buffer[2];
    blekb.keypresseddowncnt = 0;
  } else if (value.length() == 1) { // virtual joystick
    uint8_t virtjoy = (uint8_t)value[0];
    bool deactivated = virtjoy & 0x80;
    uint8_t direction = virtjoy & 0x7f;
    if (deactivated) {
      blekb.virtjoystickvalue |= (1 << direction);
    } else {
      if (direction == Joystick::C64JOYLEFT) {
        blekb.virtjoystickvalue &= ~(1 << direction);
        blekb.virtjoystickvalue |= (1 << Joystick::C64JOYRIGHT);
      } else if (direction == Joystick::C64JOYRIGHT) {
        blekb.virtjoystickvalue &= ~(1 << direction);
        blekb.virtjoystickvalue |= (1 << Joystick::C64JOYLEFT);
      } else if (direction == Joystick::C64JOYUP) {
        blekb.virtjoystickvalue &= ~(1 << direction);
        blekb.virtjoystickvalue |= (1 << Joystick::C64JOYDOWN);
      } else if (direction == Joystick::C64JOYDOWN) {
        blekb.virtjoystickvalue &= ~(1 << direction);
        blekb.virtjoystickvalue |= (1 << Joystick::C64JOYUP);
      } else { // fire
        blekb.virtjoystickvalue &= ~(1 << direction);
      }
    }
  }
}

BLEKB::BLEKB() { buffer = nullptr; }

void BLEKB::init() {
  if (buffer != nullptr) {
    // init method must be called only once
    return;
  }

  // init buffer
  buffer = new uint8_t[256];
  keypresseddowncnt = NUMOFCYCLES_KEYPRESSEDDOWN;
  bufidxprod = 0;
  bufidxcons = 0;
  kbcode1 = 0xff;
  kbcode2 = 0xff;

  // init virtual joystick
  virtjoystickvalue = 0xff;

  // init BLE
  deviceConnected = false;
  BLEDevice::init("THMIC64");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BLEKBServerCallback(*this));
  BLEService *pService = pServer->createService(Config::SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      Config::CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new BLEKBCharacteristicCallback(*this));
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
  }
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

uint8_t BLEKB::getKBJoyValue(bool port2) { return virtjoystickvalue; }

bool BLEKB::getData(uint8_t *data) {
  if (bufidxprod != bufidxcons) {
    *data = buffer[bufidxcons++];
    return true;
  }
  return false;
}
