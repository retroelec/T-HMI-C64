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
#ifndef EXTERNALCMDS_H
#define EXTERNALCMDS_H

#include "ExtCmd.h"
#include "NotificationStruct.h"
#include <cstddef>
#include <cstdint>
#include <stdexcept>

class C64Sys; // forward declaration

#include "platform/PlatformManager.h"

struct ExternalCmd {
  ExtCmd cmd;
  uint8_t param[255];
};

template <size_t size> class SimpleQueue {
private:
  ExternalCmd data[size];
  size_t head = 0;
  size_t tail = 0;
  size_t count = 0;

public:
  SimpleQueue() = default;

  bool empty() const { return count == 0; }

  void push(const ExternalCmd &value) {
    PlatformManager::getInstance().log(
        LOG_INFO, "push ExtCmd", "count = %d, cmd = %d", count, value.cmd);
    if (count == size) {
      PlatformManager::getInstance().log(LOG_ERROR, "SimpleQueue", "overflow");
      return;
    }
    data[tail] = value;
    tail = (tail + 1) % size;
    ++count;
  }

  ExternalCmd *pop() {
    if (count == 0) {
      PlatformManager::getInstance().log(LOG_ERROR, "SimpleQueue", "underflow");
      return nullptr;
    }
    ExternalCmd *value = &data[head];
    head = (head + 1) % size;
    --count;
    PlatformManager::getInstance().log(
        LOG_INFO, "pop ExtCmd", "count = %d, cmd = %d", count, value->cmd);
    return value;
  }

  ExternalCmd *waitTestAndDec() {
    if (count == 0) {
      PlatformManager::getInstance().log(LOG_ERROR, "SimpleQueue", "underflow");
      return nullptr;
    }
    ExternalCmd *cmd = &data[head];
    if ((cmd->cmd == ExtCmd::WAIT) && (cmd->param[0] > 0)) {
      cmd->param[0]--;
    }
    return cmd;
  }

  void clear() { head = tail = count = 0; }
};

class ExternalCmds {
private:
  uint8_t *ram;
  C64Sys *cpu;
  bool sendrawkeycodes;
  uint16_t actaddrreceivecmd;
  uint8_t extCmdBuf[1];
  SimpleQueue<6> extcmdQueue;

  void setType1Notification();
  void setType2Notification();
  void setType3Notification(uint16_t addr);
  void setType4Notification();
  void setType5Notification(uint8_t batteryVolLow, uint8_t batteryVolHi);
  void dispVolume();
  void writeTextToC64Screen(uint16_t addr, int16_t sizebuffer);
  bool isBasicInputMode();

public:
  bool liststartflag;

  NotificationStruct1 type1notification;
  NotificationStruct2 type2notification;
  NotificationStruct3 type3notification;
  NotificationStruct4 type4notification;
  NotificationStruct5 type5notification;

  void init(uint8_t *ram, C64Sys *cpu);
  void setVarTab(uint16_t addr);
  void queueExternalCmd(uint8_t *buffer);
  void queueExternalCmd(ExternalCmd &extcmd);
  uint8_t executeNextExternalCmd();
};

#endif // EXTERNALCMDS_H
