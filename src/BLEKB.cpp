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
#include "Config.h"
#include "Joystick.h"
#include "OSUtils.h"
#include <cstring>

static const char *TAG = "BLEKB";

static const uint8_t NUMOFCYCLES_KEYPRESSEDDOWN = 3;

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
  blekb.deviceConnected.store(true, std::memory_order_release);
  OSUtils::log(LOG_INFO, TAG, "BLE device connected");
};

void BLEKBServerCallback::onDisconnect(BLEServer *pServer) {
  blekb.deviceConnected.store(false, std::memory_order_release);
  OSUtils::log(LOG_INFO, TAG, "BLE device disconnected, try to connect...");
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
      if (direction == Joystick::C64JOYLEFT) {
        virt |= (1 << Joystick::C64JOYRIGHT);
      } else if (direction == Joystick::C64JOYRIGHT) {
        virt |= (1 << Joystick::C64JOYLEFT);
      } else if (direction == Joystick::C64JOYUP) {
        virt |= (1 << Joystick::C64JOYDOWN);
      } else if (direction == Joystick::C64JOYDOWN) {
        virt |= (1 << Joystick::C64JOYUP);
      }
    }
    blekb.virtjoystickvalue.store(virt, std::memory_order_release);
  } else if (len >= 3) { // keyboard codes or external commands
    // BLE client sends 3 codes for each key press: dc00, dc01, "shiftctrlcode"
    // BLE client sends at least 3 codes for each external command: extCmd,
    // detail, 128, {data} shiftctrlcode: bit 0 -> shift
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
  OSUtils::log(LOG_INFO, TAG, "notification sent");
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

uint8_t BLEKB::getDC01(uint8_t querydc00, bool xchgports) {
  uint8_t kbcode1 = xchgports ? sentdc01.load(std::memory_order_acquire)
                              : sentdc00.load(std::memory_order_acquire);
  uint8_t kbcode2 = xchgports ? sentdc00.load(std::memory_order_acquire)
                              : sentdc01.load(std::memory_order_acquire);
  if (querydc00 == 0) {
    return kbcode2;
  }
  // special case "shift" + "commodore"
  if ((shiftctrlcode.load(std::memory_order_acquire) & 5) == 5) {
    return (querydc00 == kbcode1) ? kbcode2 : 0xff;
  }
  // key combined with a "special key" (shift, ctrl, commodore)?
  if ((~querydc00 & 2) && (shiftctrlcode.load(std::memory_order_acquire) &
                           1)) { // *query* left shift key?
    if (kbcode1 == 0xfd) {
      // handle scan of key codes in the same "row"
      return kbcode2 & 0x7f;
    } else {
      return 0x7f;
    }
  } else if ((~querydc00 & 0x40) &&
             (shiftctrlcode.load(std::memory_order_acquire) &
              1)) { // *query* right shift key?
    if (kbcode1 == 0xbf) {
      // handle scan of key codes in the same "row"
      return kbcode2 & 0xef;
    } else {
      return 0xef;
    }
  } else if ((~querydc00 & 0x80) &&
             (shiftctrlcode.load(std::memory_order_acquire) &
              2)) { // *query* ctrl key?
    if (kbcode1 == 0x7f) {
      // handle scan of key codes in the same "row"
      return kbcode2 & 0xfb;
    } else {
      return 0xfb;
    }
  } else if ((~querydc00 & 0x80) &&
             (shiftctrlcode.load(std::memory_order_acquire) &
              4)) { // *query* commodore key?
    if (kbcode1 == 0x7f) {
      // handle scan of key codes in the same "row"
      return kbcode2 & 0xdf;
    } else {
      return 0xdf;
    }
  }
  // query key press
  return (querydc00 == kbcode1) ? kbcode2 : 0xff;
}

uint8_t BLEKB::getKBJoyValue() {
  return virtjoystickvalue.load(std::memory_order_acquire);
}

void BLEKB::setKBcodes(uint8_t sentdc01Val, uint8_t sentdc00Val) {
  sentdc01.store(sentdc01Val, std::memory_order_release);
  sentdc00.store(sentdc00Val, std::memory_order_release);
}

void BLEKB::setDetectReleasekey(bool detectreleasekeyVal) {
  detectreleasekey.store(detectreleasekeyVal, std::memory_order_release);
}
