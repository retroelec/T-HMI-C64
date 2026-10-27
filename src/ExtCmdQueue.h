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
#include <atomic>
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

  bool empty() const { return count.load(std::memory_order_acquire) == 0; }

  void push(const ExternalCmd &value) {
    // PlatformManager::getInstance().log(
    //     LOG_INFO, "push ExtCmd", "count = %d, cmd = %d",
    //     count.load(), value.cmd);
    if (count.load(std::memory_order_relaxed) == SIZE) {
      PlatformManager::getInstance().log(LOG_ERROR, "ExtCmdQueue", "overflow");
      return;
    }
    uint8_t t = tail.load(std::memory_order_relaxed);
    data[t] = value;
    tail.store((t + 1) % SIZE, std::memory_order_relaxed);
    count.fetch_add(1, std::memory_order_release);
  }

  ExternalCmd *pop() {
    if (count.load(std::memory_order_relaxed) == 0) {
      PlatformManager::getInstance().log(LOG_ERROR, "ExtCmdQueue", "underflow");
      return nullptr;
    }
    uint8_t h = head.load(std::memory_order_relaxed);
    ExternalCmd *value = &data[h];
    head.store((h + 1) % SIZE, std::memory_order_relaxed);
    count.fetch_sub(1, std::memory_order_acquire);
    // PlatformManager::getInstance().log(
    //     LOG_INFO, "pop ExtCmd", "count = %d, cmd = %d", count.load(),
    //     value->cmd);
    return value;
  }

  ExternalCmd *waitTestAndDec() {
    if (count.load(std::memory_order_relaxed) == 0) {
      PlatformManager::getInstance().log(LOG_ERROR, "ExtCmdQueue", "underflow");
      return nullptr;
    }
    uint8_t h = head.load(std::memory_order_relaxed);
    ExternalCmd *cmd = &data[h];
    if ((cmd->cmd == ExtCmd::WAIT) && (cmd->param[0] > 0)) {
      cmd->param[0]--;
    }
    return cmd;
  }

  void clear() {
    head.store(0);
    tail.store(0);
    count.store(0, std::memory_order_release);
  }

private:
  ExtCmdQueue() = default;
  static const uint8_t SIZE = 6;
  ExternalCmd data[SIZE];
  std::atomic<uint8_t> head = 0;
  std::atomic<uint8_t> tail = 0;
  std::atomic<uint8_t> count = 0;
};

#endif // EXTCMDQUEUE_H
