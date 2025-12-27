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
#include "BLEKB.h"
#include "../Config.h"
#ifdef USE_BLE_KEYBOARD
#include "../platform/PlatformManager.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <atomic>
#include <cstdint>

static const char *TAG = "BLEKB";

static const uint8_t NUMOFCYCLES_KEYPRESSEDDOWN = 3;

static const uint8_t C64JOYUP = 0;
static const uint8_t C64JOYDOWN = 1;
static const uint8_t C64JOYLEFT = 2;
static const uint8_t C64JOYRIGHT = 3;
static const uint8_t C64JOYFIRE = 4;

BLEKBServerCallback::BLEKBServerCallback(BLEKB &blekb) : blekb(blekb) {}

void BLEKBServerCallback::onConnect(BLEServer *pServer) {
  blekb.deviceConnected.store(true, std::memory_order_release);
  PlatformManager::getInstance().log(LOG_INFO, TAG, "BLE device connected");
};

void BLEKBServerCallback::onDisconnect(BLEServer *pServer) {
  blekb.deviceConnected.store(false, std::memory_order_release);
  PlatformManager::getInstance().log(
      LOG_INFO, TAG, "BLE device disconnected, try to connect...");
  pServer->getAdvertising()->start();
}

BLEKBCharacteristicCallback::BLEKBCharacteristicCallback(BLEKB &blekb)
    : blekb(blekb) {}

void BLEKBCharacteristicCallback::onWrite(BLECharacteristic *pCharacteristic) {
  if (!blekb.deviceConnected.load(std::memory_order_acquire)) {
    return;
  }

  String value = pCharacteristic->getValue();
  uint8_t len = value.length() > 255 ? 255 : value.length();

  if (len == 1) { // virtual joystick or release key
    uint8_t virtjoy = (uint8_t)value[0];

    if (virtjoy == 0xff) {
      // key released
      blekb.keypresseddown.store(false, std::memory_order_release);
    }

    bool deactivated = virtjoy & 0x80;
    uint8_t direction = virtjoy & 0x7f;
    uint8_t virt = blekb.virtjoystickvalue.load(std::memory_order_acquire);
    if (deactivated) {
      virt |= (1 << direction);
    } else {
      virt &= ~(1 << direction);
      if (direction == C64JOYLEFT) {
        virt |= (1 << C64JOYRIGHT);
      } else if (direction == C64JOYRIGHT) {
        virt |= (1 << C64JOYLEFT);
      } else if (direction == C64JOYUP) {
        virt |= (1 << C64JOYDOWN);
      } else if (direction == C64JOYDOWN) {
        virt |= (1 << C64JOYUP);
      }
    }
    blekb.virtjoystickvalue.store(virt, std::memory_order_release);
  } else if (len >= 3) { // keyboard codes or external commands
    // BLE client sends 3 codes for each key press: dc00, dc01, "shiftctrlcode"
    // BLE client sends at least 3 codes for each external command:
    // extCmd, detail, 128, {data}
    // shiftctrlcode: bit 0 -> shift
    //                bit 1 -> ctrl
    //                bit 2 -> commodore
    //                bit 7 -> external command
    for (uint8_t i = 0; i < len; i++) {
      blekb.buffer[i] = (uint8_t)value[i];
    }
    blekb.shiftctrlcode.store(blekb.buffer[2], std::memory_order_release);
    if (!(blekb.shiftctrlcode & 128)) {
      blekb.keypresseddowncnt.store(0, std::memory_order_release);
      blekb.keypresseddown.store(true, std::memory_order_release);
    }
  }
}

BLEKB::BLEKB() { buffer = nullptr; }

void BLEKB::init() {
  if (buffer != nullptr) {
    return;
  }
  buffer = new uint8_t[256];
  keypresseddowncnt.store(NUMOFCYCLES_KEYPRESSEDDOWN,
                          std::memory_order_release);
  keypresseddown.store(false, std::memory_order_release);
  sentdc01.store(0xff, std::memory_order_release);
  sentdc00.store(0xff, std::memory_order_release);
  virtjoystickvalue.store(0xff, std::memory_order_release);
  detectreleasekey.store(true, std::memory_order_release);

  // init BLE
  deviceConnected.store(false, std::memory_order_release);
  BLEDevice::init("THMIC64");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BLEKBServerCallback(*this));

  BLEService *pService = pServer->createService(Config::SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      Config::CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->setCallbacks(new BLEKBCharacteristicCallback(*this));
  pCharacteristic->setValue("THMIC64");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(Config::SERVICE_UUID);
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->setMinInterval(0x30); // min advertising interval (48*0.625ms)
  pAdvertising->setMaxInterval(0x60); // max advertising interval (96*0.625ms)
  pAdvertising->start();

  esp_ble_conn_update_params_t conn_params = {
      .bda = {},
      .min_int = 0x30, // min connection interval (0x30 x 1.25ms = 60ms)
      .max_int = 0x50, // max connection interval (0x50 x 1.25ms = 100ms)
      .latency = 0,    // slave latency
      .timeout = 400   // connection supervision timeout (400 x 10ms = 4s)
  };
  esp_ble_gap_update_conn_params(&conn_params);
}

uint8_t *BLEKB::getExtCmdData() {
  if (shiftctrlcode.load(std::memory_order_acquire) & 128) {
    shiftctrlcode.store(0, std::memory_order_release);
    sentdc01.store(0xff, std::memory_order_release);
    sentdc00.store(0xff, std::memory_order_release);
    keypresseddowncnt.store(NUMOFCYCLES_KEYPRESSEDDOWN,
                            std::memory_order_release);
    keypresseddown.store(false, std::memory_order_release);
    return buffer;
  }
  return nullptr;
}

void BLEKB::sendExtCmdNotification(uint8_t *data, size_t size) {
  pCharacteristic->setValue(data, size);
  pCharacteristic->notify();
  PlatformManager::getInstance().log(LOG_INFO, TAG, "notification sent");
}

void BLEKB::scanKeyboard() {
  // "external" commands are handled in getExtCmdData()
  if (!(shiftctrlcode.load(std::memory_order_acquire) & 128)) {
    if ((detectreleasekey.load(std::memory_order_acquire) &&
         keypresseddown.load(std::memory_order_acquire)) ||
        (keypresseddowncnt.load(std::memory_order_acquire) <
         NUMOFCYCLES_KEYPRESSEDDOWN)) {
      // id detectreleasekey == false: key is "pressed down" at least for 24 ms
      sentdc00.store(buffer[0], std::memory_order_release);
      sentdc01.store(buffer[1], std::memory_order_release);
      keypresseddowncnt.store(
          keypresseddowncnt.load(std::memory_order_acquire) + 1,
          std::memory_order_release);
    } else {
      sentdc01.store(0xff, std::memory_order_release);
      sentdc00.store(0xff, std::memory_order_release);
    }
  }
}

uint8_t BLEKB::getKBCodeDC01() { return sentdc01; }

uint8_t BLEKB::getKBCodeDC00() { return sentdc00; }

uint8_t BLEKB::getShiftctrlcode() { return shiftctrlcode; }

uint8_t BLEKB::getKBJoyValue() {
  return virtjoystickvalue.load(std::memory_order_acquire);
}

void BLEKB::setDetectReleasekey(bool detectreleasekeyVal) {
  detectreleasekey.store(detectreleasekeyVal, std::memory_order_release);
}
#endif
