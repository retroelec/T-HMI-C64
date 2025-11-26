#ifndef WEBKB_H
#define WEBKB_H

#include "../Config.h"
#ifdef USE_WEB_KEYBOARD

#include "KeyboardDriver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <atomic>
#include <queue>

using CodeTriple = std::tuple<uint8_t, uint8_t, uint8_t>;

struct ActiveKey {
  uint8_t dc1;
  uint8_t dc0;
  uint8_t shift;
  uint8_t holdTicks;
  bool active;
};

class WebKB : public KeyboardDriver {
public:
  WebKB(uint16_t port = 80);
  ~WebKB();

  void init() override;

  uint8_t getKBCodeDC01() override {
    uint16_t v = dc01dc00.load(std::memory_order_acquire);
    return static_cast<uint8_t>(v >> 8);
  }
  uint8_t getKBCodeDC00() override {
    uint16_t v = dc01dc00.load(std::memory_order_acquire);
    return static_cast<uint8_t>(v & 0xFF);
  }
  uint8_t getShiftctrlcode() override {
    return shiftctrlcode.load(std::memory_order_acquire);
  }
  uint8_t getKBJoyValue() override {
    return joyvalue.load(std::memory_order_acquire);
  }

  uint8_t *getExtCmdData() override;
  void sendExtCmdNotification(uint8_t *data, size_t size) override;

  void scanKeyboard() override;
  void setDetectReleasekey(bool detectreleasekey) override {}

  void setKBcodes(uint8_t sentdc01, uint8_t sentdc00) override {
    uint16_t packed = (static_cast<uint16_t>(sentdc01) << 8) | sentdc00;
    dc01dc00.store(packed, std::memory_order_release);
  }

private:
  void startWebServer();
  void handleWebsocketMessage(void *arg, uint8_t *data, size_t len);
  void processSingleKey(const char *type, const char *keyId, bool shift,
                        bool ctrl, bool comm);

  uint16_t port;
  AsyncWebServer *server;
  AsyncWebSocket *ws;

  std::atomic<uint16_t> dc01dc00; // high byte = dc01, low byte = dc00
  std::atomic<uint8_t> shiftctrlcode{0};
  std::atomic<uint8_t> joyvalue{0};
  std::atomic<bool> gotExternalCmd = false;
  uint8_t extCmdBuffer[1024];
  bool shiftlock = false;
  std::queue<CodeTriple> eventQueue;
  SemaphoreHandle_t queueSem;
  ActiveKey currentKey;
};

#endif

#endif
