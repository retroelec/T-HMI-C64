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
#ifndef EXTCMDQUEUE_H
#define EXTCMDQUEUE_H

#include "ExtCmd.h"
#include "platform/PlatformManager.h"
#include <cstdint>
#include <cstring>

/**
 * @brief Singleton to push and pop external commands.
 *
 * This class implements a queue to manage external commands.
 */
class ExtCmdQueue {
public:
  struct ExternalCmd {
    ExtCmd cmd;
    uint8_t param[255];
  };

  static ExtCmdQueue &getInstance() {
    static ExtCmdQueue instance;
    return instance;
  }
  ExtCmdQueue(const ExtCmdQueue &) = delete;
  ExtCmdQueue &operator=(const ExtCmdQueue &) = delete;

  bool empty() const { return count == 0; }

  void push(const ExternalCmd &value) {
    PlatformManager::getInstance().log(
        LOG_INFO, "push ExtCmd", "count = %d, cmd = %d", count, value.cmd);
    if (count == SIZE) {
      PlatformManager::getInstance().log(LOG_ERROR, "ExtCmdQueue", "overflow");
      return;
    }
    data[tail] = value;
    tail = (tail + 1) % SIZE;
    ++count;
  }

  ExternalCmd *pop() {
    if (count == 0) {
      PlatformManager::getInstance().log(LOG_ERROR, "ExtCmdQueue", "underflow");
      return nullptr;
    }
    ExternalCmd *value = &data[head];
    head = (head + 1) % SIZE;
    --count;
    PlatformManager::getInstance().log(
        LOG_INFO, "pop ExtCmd", "count = %d, cmd = %d", count, value->cmd);
    return value;
  }

  ExternalCmd *waitTestAndDec() {
    if (count == 0) {
      PlatformManager::getInstance().log(LOG_ERROR, "ExtCmdQueue", "underflow");
      return nullptr;
    }
    ExternalCmd *cmd = &data[head];
    if ((cmd->cmd == ExtCmd::WAIT) && (cmd->param[0] > 0)) {
      cmd->param[0]--;
    }
    return cmd;
  }

  void clear() { head = tail = count = 0; }

private:
  ExtCmdQueue() = default;
  static const uint8_t SIZE = 6;
  ExternalCmd data[SIZE];
  uint8_t head = 0;
  uint8_t tail = 0;
  uint8_t count = 0;
};

#endif // EXTCMDQUEUE_H
