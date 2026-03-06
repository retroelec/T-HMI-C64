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

#undef CONFIG_LOG_MAXIMUM_LEVEL
#define CONFIG_LOG_MAXIMUM_LEVEL 5

#include "Config.h"
#include "ExtCmd.h"
#include "board/BoardDriver.h"
#include "board/BoardFactory.h"
#include "joystick/JoystickDriver.h"
#include "joystick/JoystickFactory.h"
#include "platform/PlatformFactory.h"
#include "platform/PlatformManager.h"
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <cstdint>

static const char *TAG = "BLEJoystick";

static boolean connected = false;
static boolean needsRescan = false;
static BLEClient *pClient = nullptr;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;
static BoardDriver *board;
static JoystickDriver *joystick;
static uint8_t oldJoyVal = 0xff;
static uint8_t cnt = 5;
static ExtCmd actJoyMode = ExtCmd::KBJOYSTICKMODE2;
static uint8_t cntfire2 = 0;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(BLEUUID(Config::SERVICE_UUID))) {
      BLEDevice::getScan()->stop();
      if (myDevice != nullptr) {
        delete myDevice;
      }
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      connected = false;
    }
  }
};

class MyClientCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient *pClient) override {}

  void onDisconnect(BLEClient *pClient) override {
    connected = false;
    needsRescan = true;
    pRemoteCharacteristic = nullptr;
    PlatformManager::getInstance().log(LOG_INFO, TAG,
                                       "disconnected from BLE server");
  }
};

void initBLE() {
  BLEDevice::init("ESP32-S3-Joystick");
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(100);
  pBLEScan->start(0, false);
}

void setup() {

  Serial.begin(115200);
  delay(500);

  // init platform
  PlatformManager::initialize(PlatformNS::create());
  PlatformManager::getInstance().log(LOG_INFO, TAG, "start setup...");

  // init board
  board = Board::create();
  board->init();

  // init joystick
  joystick = Joystick::create();
  joystick->init();

  initBLE();

  PlatformManager::getInstance().log(LOG_INFO, TAG, "setup finished");
}

bool connectToServer() {
  PlatformManager::getInstance().log(LOG_INFO, TAG, "connect to %s",
                                     myDevice->getAddress().toString().c_str());
  if (pClient == nullptr) {
    pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallbacks());
  }
  if (pClient->isConnected())
    return true;
  if (!pClient->connect(myDevice)) {
    return false;
  }
  delay(300);
  BLERemoteService *pRemoteService =
      pClient->getService(BLEUUID(Config::SERVICE_UUID));
  if (pRemoteService == nullptr) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "service not found");
    pClient->disconnect();
    return false;
  }
  pRemoteCharacteristic =
      pRemoteService->getCharacteristic(BLEUUID(Config::CHARACTERISTIC_UUID));
  if (pRemoteCharacteristic == nullptr) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                       "characteristic not found");
    pClient->disconnect();
    return false;
  }
  PlatformManager::getInstance().log(LOG_INFO, TAG, "connected to BLE server");
  return true;
}

void sendByte(uint8_t val) {
  if (connected && pRemoteCharacteristic != nullptr) {
    if (!pClient->isConnected()) {
      connected = false;
      needsRescan = true;
      return;
    }
    pRemoteCharacteristic->writeValue(&val, 1, true);
  }
}

void sendBytes(const uint8_t *data, size_t length) {
  if (connected && pRemoteCharacteristic != nullptr) {
    if (!pClient->isConnected()) {
      connected = false;
      needsRescan = true;
      return;
    }
    pRemoteCharacteristic->writeValue(const_cast<uint8_t *>(data), length,
                                      true);
  }
}

void loop() {
  if (needsRescan) {
    needsRescan = false;
    delay(300);
    PlatformManager::getInstance().log(LOG_INFO, TAG, "start rescan...");
    BLEDevice::getScan()->start(0, false);
  }

  if (!connected) {
    delay(300);
    if (connectToServer()) {
      connected = true;
      PlatformManager::getInstance().log(LOG_INFO, TAG,
                                         "set BLE joystick port");
      uint8_t cmd[3];
      cmd[0] = static_cast<uint8_t>(actJoyMode);
      cmd[2] = 0x80;
      sendBytes(cmd, 3);
    }
  } else {
    if (joystick->getFire2()) {
      cntfire2++;
      if (cntfire2 == 200) {
        if (actJoyMode == ExtCmd::KBJOYSTICKMODE2) {
          PlatformManager::getInstance().log(LOG_INFO, TAG,
                                             "change BLE joystick port to 1");
          actJoyMode = ExtCmd::KBJOYSTICKMODE1;
        } else {
          PlatformManager::getInstance().log(LOG_INFO, TAG,
                                             "change BLE joystick port to 2");
          actJoyMode = ExtCmd::KBJOYSTICKMODE2;
        }
        uint8_t cmd[3];
        cmd[0] = static_cast<uint8_t>(actJoyMode);
        cmd[2] = 0x80;
        sendBytes(cmd, 3);
      }
    } else {
      cntfire2 = 0;
    }
    uint8_t joyVal = joystick->getValue();
    cnt--;
    if ((joyVal != oldJoyVal) || (cnt == 0)) {
      cnt = 5;
      oldJoyVal = joyVal;
      // set bit 6 zo zero to distinguish a key release from joystick
      sendByte(joyVal & 0xbf);
    }
    delay(10);
  }
}
