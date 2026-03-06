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
#include "../Config.h"
#ifdef USE_BLE_KEYBOARD
#include "../ExtCmdQueue.h"
#include "../platform/PlatformManager.h"
#include "BLEKB.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <atomic>
#include <cstdint>

static const char *TAG = "BLEKB";

static const uint8_t NUMOFCYCLES_KEYPRESSEDDOWN = 3;

BLEKBServerCallback::BLEKBServerCallback(BLEKB &blekb) : blekb(blekb) {}

static uint8_t bleconnectedbox[] = "\x55\x43\x43\x43\x43\x43\x43\x43\x43\x43"
                                   "\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43"
                                   "\x43\x49"
                                   "\x42\x02\x0c\x05\x20\x04\x05\x16\x09\x03"
                                   "\x05\x20\x03\x0f\x0e\x0e\x05\x03\x14\x05"
                                   "\x04\x42"
                                   "\x4a\x43\x43\x43\x43\x43\x43\x43\x43\x43"
                                   "\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43"
                                   "\x43\x4b";

void BLEKBServerCallback::onConnect(BLEServer *pServer) {
  blekb.deviceConnected.store(true, std::memory_order_release);
  PlatformManager::getInstance().log(LOG_INFO, TAG, "BLE device connected");
  ExtCmdQueue::ExternalCmd extcmd;
  extcmd.param[2] = 6;
  extcmd.param[3] = 5;
  extcmd.param[4] = 22;
  extcmd.param[5] = 3;
  extcmd.param[6] = 1;
  extcmd.param[7] = 0;
  extcmd.param[8] = 5;
  extcmd.param[9] = 0;
  extcmd.param[10] = 1;
  memcpy(&extcmd.param[11], bleconnectedbox, 22 * 3);
  extcmd.cmd = ExtCmd::WRITEOSD;
  ExtCmdQueue::getInstance().push(extcmd);
  // continue advertising so we can connect to multiple clients
  pServer->getAdvertising()->start();
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
    } else {
      // bit 6 was cleared from BLEClient to distinguish between a key release
      // and the virtual joystick add bit 6 again
      virtjoy |= 0x40;
      blekb.virtjoystickvalue.store(virtjoy, std::memory_order_release);
    }
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

void BLEKB::sendExtCmdNotification(uint8_t *data, size_t size) {
  pCharacteristic->setValue(data, size);
  pCharacteristic->notify();
  PlatformManager::getInstance().log(LOG_INFO, TAG, "notification sent");
}

void BLEKB::scanKeyboard() {
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
  } else {
    shiftctrlcode.store(0, std::memory_order_release);
    sentdc01.store(0xff, std::memory_order_release);
    sentdc00.store(0xff, std::memory_order_release);
    keypresseddowncnt.store(NUMOFCYCLES_KEYPRESSEDDOWN,
                            std::memory_order_release);
    keypresseddown.store(false, std::memory_order_release);
    ExtCmdQueue::ExternalCmd extcmd;
    extcmd.cmd = static_cast<ExtCmd>(buffer[0]);
    memcpy(&extcmd.param[0], &buffer[1], 255);
    ExtCmdQueue::getInstance().push(extcmd);
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
