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
#include "ExternalCmds.h"
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

class ExeExtCmdTask {
private:
  TaskHandle_t exeExtCmdTaskHandle;
  BLEKB &blekb;
  ExternalCmds &externalCmds;
  uint8_t type;

  static void exeExternalCmd(void *parameter) {
    ExeExtCmdTask *instance = static_cast<ExeExtCmdTask *>(parameter);
    instance->type =
        instance->externalCmds.executeExternalCmd(instance->blekb.buffer);
    // signal that task is done using a task notification
    BaseType_t notifyResult = xTaskNotifyGive(instance->exeExtCmdTaskHandle);
    if (notifyResult == pdPASS) {
      ESP_LOGD(TAG, "Notification sent successfully");
    } else {
      ESP_LOGD(TAG, "Failed to send notification");
    }
    // delete task
    vTaskDelete(NULL);
  }

public:
  ExeExtCmdTask(BLEKB &blekb, ExternalCmds &externalCmds)
      : exeExtCmdTaskHandle(NULL), blekb(blekb), externalCmds(externalCmds) {}

  uint8_t exe() {
    exeExtCmdTaskHandle = xTaskGetCurrentTaskHandle();
    xTaskCreate(ExeExtCmdTask::exeExternalCmd, // Task function
                "externalCmdTask",             // Name of the task
                10000,                         // Stack size (in words)
                this,                          // Task input parameter
                1,                             // Priority of the task
                NULL                           // Task handle
    );
    // wait for task to be completed
    uint32_t ulNotificationValue;
    BaseType_t result = xTaskNotifyWait(
        0x00,                 // No bits to clear on entry
        ULONG_MAX,            // Clear all bits on exit
        &ulNotificationValue, // Where to store the notification value
        pdMS_TO_TICKS(5000)   // Wait max 5 seconds
    );
    if (result == pdPASS) {
      // Return the stored return value
      ESP_LOGD(TAG, "task completed, type = %d", type);
      return type;
    } else {
      ESP_LOGD(TAG, "failed to send notification or timed out");
      return 0;
    }
  }
};

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

BLEKBCharacteristicCallback::BLEKBCharacteristicCallback(
    BLEKB &blekb, ExternalCmds &externalCmds)
    : blekb(blekb), externalCmds(externalCmds) {}

void BLEKBCharacteristicCallback::onWrite(BLECharacteristic *pCharacteristic) {
  if (!blekb.deviceConnected) {
    return;
  }
  std::string value = pCharacteristic->getValue();
  uint8_t len = value.length() > 255 ? 255 : value.length();
  if (len == 1) { // virtual joystick
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
  } else if (len >= 3) { // keyboard codes or external commands
    // BLE client sends 3 codes for each key: dc00, dc01, "shiftctrlcode"
    // shiftctrlcode: bit 0 -> left shift
    //                bit 1 -> ctrl
    //                bit 2 -> commodore
    //                bit 7 -> external command (cmd, x, 128)
    for (uint8_t i = 0; i < len; i++) {
      blekb.buffer[i] = (uint8_t)value[i];
    }
    blekb.shiftctrlcode = blekb.buffer[2];
    blekb.keypresseddowncnt = 0;
    // external command?
    if (blekb.buffer[2] & 128) {
      // execute external command
      // have to use a seperate task, because stack size of the BLE processing
      // task is too small (and task size is not changeable uner Arduino)
      ESP_LOGI(TAG, "run external command...");
      ExeExtCmdTask exeExtCmdTask(blekb, externalCmds);
      uint8_t type = exeExtCmdTask.exe();
      // send notification
      switch (type) {
      case 1:
        pCharacteristic->setValue(
            reinterpret_cast<uint8_t *>(&(externalCmds.type1notification)),
            sizeof(BLENotificationStruct1));
        break;
      case 2:
        pCharacteristic->setValue(
            reinterpret_cast<uint8_t *>(&(externalCmds.type2notification)),
            sizeof(BLENotificationStruct2));
        break;
      case 3:
        pCharacteristic->setValue(
            reinterpret_cast<uint8_t *>(&(externalCmds.type3notification)),
            sizeof(BLENotificationStruct3));
        break;
      }
      pCharacteristic->notify();
    }
  }
}

BLEKB::BLEKB() { buffer = nullptr; }

void BLEKB::init(ExternalCmds *externalCmds) {
  if (buffer != nullptr) {
    // init method must be called only once
    return;
  }

  // init buffer
  buffer = new uint8_t[256];
  keypresseddowncnt = NUMOFCYCLES_KEYPRESSEDDOWN;
  kbcode1 = 0xff;
  kbcode2 = 0xff;

  // init div
  virtjoystickvalue = 0xff;

  // init BLE
  deviceConnected = false;
  BLEDevice::init("THMIC64");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BLEKBServerCallback(*this));
  BLEService *pService = pServer->createService(Config::SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      Config::CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->setCallbacks(
      new BLEKBCharacteristicCallback(*this, *externalCmds));
  pCharacteristic->setValue("THMIC64");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void BLEKB::handleKeyPress() {
  if ((shiftctrlcode & 128) != 0) {
    // "external" commands are handled in BLEKBCharacteristicCallback::onWrite
    shiftctrlcode = 0;
    kbcode1 = 0xff;
    keypresseddowncnt = NUMOFCYCLES_KEYPRESSEDDOWN;
    return;
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
  return;
}

uint8_t BLEKB::decode(uint8_t dc00) {
  if (dc00 == 0) {
    return kbcode1;
  }
  if ((~dc00 & 2) && (shiftctrlcode & 1)) {
    // left shift pressed
    if (kbcode2 == 0xfd) {
      // handle scan of key codes in the same "row"
      return kbcode1 & 0x7f;
    } else {
      return 0x7f;
    }
  } else if ((~dc00 & 0x80) && (shiftctrlcode & 2)) {
    // ctrl pressed
    if (kbcode2 == 0x7f) {
      // handle scan of key codes in the same "row"
      return kbcode1 & 0xfb;
    } else {
      return 0xfb;
    }
  } else if ((~dc00 & 0x80) && (shiftctrlcode & 4)) {
    // commodore pressed
    if (kbcode2 == 0x7f) {
      // handle scan of key codes in the same "row"
      return kbcode1 & 0xdf;
    } else {
      return 0xdf;
    }
  }
  if (dc00 == kbcode2) {
    return kbcode1;
  } else {
    return 0xff;
  }
}

uint8_t BLEKB::getKBJoyValue(bool port2) { return virtjoystickvalue; }

void BLEKB::setKbcodes(uint8_t kbcode1, uint8_t kbcode2) {
  this->kbcode1 = kbcode1;
  this->kbcode2 = kbcode2;
}
