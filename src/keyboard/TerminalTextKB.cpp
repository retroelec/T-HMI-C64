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
#ifdef USE_TERMINALTEXT_KEYBOARD

#include "../ExtCmd.h"
#include "../ExtCmdQueue.h"
#include "NotcursesKeyboardEvent.h"
#include "TerminalTextKB.h"
#include <cstdint>
#include <cstring>
#include <optional>
#include <poll.h>
#include <unistd.h>

TerminalTextKB::TerminalTextKB()
    : kbcode1(0xff), kbcode2(0xff), shiftctrlcode(0) {}
TerminalTextKB::~TerminalTextKB() {
  // Restore terminal to original settings
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
}

void TerminalTextKB::setCodes(uint8_t code1, uint8_t code2, uint8_t ctrlcode) {
  kbcode1.store(code1, std::memory_order_release);
  kbcode2.store(code2, std::memory_order_release);
  shiftctrlcode.store(ctrlcode, std::memory_order_release);
}

std::optional<CodeTripleS> TerminalTextKB::getKeyCodes(uint32_t key, bool shift,
                                                       bool alt) {
  auto &map = keyboardLayout->getMapping();
  KeySpec spec{key, shift, alt};
  auto it = map.find(spec);
  if (it != map.end()) {
    return it->second;
  }
  return std::nullopt;
}

void TerminalTextKB::handleKeyEvent(uint32_t ch, bool shift, bool ctrl,
                                    bool alt, bool pressed) {
  ctrlDown = ctrl;

  uint32_t key = keyboardLayout->normalizeKey(ch);
  if (key != ch && !alt) {
    shift = true;
  }
  if (ch >= 'A' && ch <= 'Z') {
    shift = true;
  }

  if (pressed && ctrl) {
    ExtCmdQueue::ExternalCmd extcmd{};
    extcmd.cmd = ExtCmd::NOEXTCMD;
    switch (ch) {
    case 'h':
    case 'H': {
      static const uint8_t help[] = "\x93\x5\r"
                                    "          **** HELP PAGE ****\r\r"
                                    "CTRL-H FOR THIS HELP PAGE\r"
                                    "CTRL-Q TO QUIT THE EMULATOR\r"
                                    "CTRL-L TO LOAD A PROGRAM\r"
                                    "CTRL-S TO SAVE A PROGRAM\r"
                                    "CTRL-T TO LIST PROGRAMS\r"
                                    "CTRL-R TO RESET THE EMULATOR\r"
                                    "CTRL-D TO DECREMENT SOUND VOLUME\r"
                                    "CTRL-U TO INCREMENT SOUND VOLUME\r"
                                    "CTRL-K TO SWITCH BETWEEN JOYSTICK\r"
                                    "       IN PORT 1, IN PORT 2, NO\r"
                                    "       JOYSTICK\r"
                                    "CTRL-P TO PAUSE\r"
                                    "CTRL-X TO SWITCH DISPLAY MODE\r\x9a\0";
      extcmd.cmd = ExtCmd::WRITETEXT;
      int16_t helpsize = sizeof(help);
      uint8_t *helpptr = (uint8_t *)help;
      while (helpsize > 0) {
        uint8_t numBytes = (helpsize > 250) ? 250 : helpsize;
        memcpy(&extcmd.param[2], helpptr, numBytes);
        extcmd.param[2 + numBytes] = '\0';
        ExtCmdQueue::getInstance().push(extcmd);
        helpsize -= 250;
        helpptr += 250;
      }
      return;
    }
    case 'q':
    case 'Q':
      exit(0);
    case 'l':
    case 'L':
      extcmd.cmd = ExtCmd::LOAD;
      break;
    case 's':
    case 'S':
      extcmd.cmd = ExtCmd::SAVE;
      break;
    case 'r':
    case 'R':
      extcmd.cmd = ExtCmd::RESET;
      break;
    case 't':
    case 'T':
      extcmd.cmd = ExtCmd::LIST;
      break;
    case 'd':
    case 'D':
      extcmd.cmd = ExtCmd::DECVOLUME;
      extcmd.param[0] = 10;
      break;
    case 'u':
    case 'U':
      extcmd.cmd = ExtCmd::INCVOLUME;
      extcmd.param[0] = 10;
      break;
    case 'p':
    case 'P':
      extcmd.cmd = ExtCmd::PAUSE;
      break;
    case 'x':
    case 'X':
      extcmd.cmd = ExtCmd::SPECIAL1;
      break;
    case 'k':
    case 'K':
      switch (joystickmode) {
      case ExtCmd::JOYSTICKMODEOFF:
        joystickmode = ExtCmd::JOYSTICKMODE1;
        extcmd.cmd = ExtCmd::JOYSTICKMODE1;
        break;
      case ExtCmd::JOYSTICKMODE1:
        joystickmode = ExtCmd::JOYSTICKMODE2;
        extcmd.cmd = ExtCmd::JOYSTICKMODE2;
        break;
      case ExtCmd::JOYSTICKMODE2:
        joystickmode = ExtCmd::JOYSTICKMODEOFF;
        extcmd.cmd = ExtCmd::JOYSTICKMODEOFF;
        break;
      default:
        break;
      }
      ExtCmdQueue::getInstance().push(extcmd);
      return;
    default:
      break;
    }
    if (extcmd.cmd != ExtCmd::NOEXTCMD) {
      ExtCmdQueue::getInstance().push(extcmd);
      return;
    }
    // fall through: non-ExtCmd Ctrl key → normal key processing
  }

  bool lctrl = ctrl;
  bool rctrl = false;
  bool lalt = false;
  bool ralt = alt;
  auto getKc = [this](uint32_t k, bool s, bool a) {
    return getKeyCodes(k, s, a);
  };
  auto setCd = [this](uint8_t c1, uint8_t c2, uint8_t c3) {
    setCodes(c1, c2, c3);
  };
  processKeyEvent(key, shift, lctrl, rctrl, lalt, ralt, pressed, getKc, setCd);
}

static bool hasStdinData() {
  struct pollfd pfd;
  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;
  pfd.revents = 0;
  return poll(&pfd, 1, 0) > 0;
}

uint32_t TerminalTextKB::readKey() {
  uint8_t buf[8];
  ssize_t n = read(STDIN_FILENO, buf, 1);
  if (n <= 0) {
    return 0;
  }

  // regular ASCII
  if (buf[0] != 0x1B) {
    if (buf[0] == 0x0A || buf[0] == 0x0D) {
      return TKEY_ENTER;
    }
    if (buf[0] == 0x7F) {
      return TKEY_BACKSPACE;
    }
    if (buf[0] == 0x09) {
      return TKEY_TAB;
    }
    return buf[0];
  }

  // escape sequence: check if there are more bytes (non-blocking)
  if (!hasStdinData()) {
    return TKEY_ESC;
  }
  n = read(STDIN_FILENO, buf, 1);
  if (n <= 0) {
    return TKEY_ESC;
  }

  if (buf[0] == '[') {
    if (!hasStdinData()) {
      return TKEY_ESC;
    }
    n = read(STDIN_FILENO, buf, 1);
    if (n <= 0) {
      return TKEY_ESC;
    }

    // CSI sequences: ESC [ char or ESC [ number ~
    if (buf[0] >= '0' && buf[0] <= '9') {
      uint8_t num = buf[0] - '0';
      while (true) {
        if (!hasStdinData()) {
          return TKEY_ESC;
        }
        n = read(STDIN_FILENO, buf, 1);
        if (n <= 0) {
          return TKEY_ESC;
        }
        if (buf[0] >= '0' && buf[0] <= '9') {
          num = num * 10 + (buf[0] - '0');
        } else if (buf[0] == '~') {
          break;
        } else {
          return TKEY_ESC;
        }
      }
      switch (num) {
      case 1:
        return TKEY_HOME;
      case 2:
        return TKEY_INS;
      case 3:
        return TKEY_DEL;
      case 5:
        return 0; // PGUP - ignore
      case 6:
        return 0; // PGDN - ignore
      case 15:
        return TKEY_F1; // xterm F5
      case 17:
        return TKEY_F3; // xterm F6
      case 18:
        return TKEY_F5; // xterm F7
      case 19:
        return TKEY_F7; // xterm F8
      default:
        return 0; // ignore
      }
    } else {
      switch (buf[0]) {
      case 'A':
        return TKEY_UP;
      case 'B':
        return TKEY_DOWN;
      case 'C':
        return TKEY_RIGHT;
      case 'D':
        return TKEY_LEFT;
      case 'H':
        return TKEY_HOME;
      case 'F':
        return 0; // END - ignore
      case 'Z':
        return TKEY_TAB; // Shift+Tab
      default:
        return TKEY_ESC;
      }
    }
  } else if (buf[0] == 'O') {
    if (!hasStdinData()) {
      return TKEY_ESC;
    }
    n = read(STDIN_FILENO, buf, 1);
    if (n <= 0) {
      return TKEY_ESC;
    }
    switch (buf[0]) {
    case 'P':
      return TKEY_F1;
    case 'Q':
      return TKEY_F3;
    case 'R':
      return TKEY_F5;
    case 'S':
      return TKEY_F7;
    default:
      return TKEY_ESC;
    }
  }

  return TKEY_ESC;
}

void TerminalTextKB::init() {
  kbcode1.store(0xff, std::memory_order_release);
  kbcode2.store(0xff, std::memory_order_release);
  shiftctrlcode.store(0, std::memory_order_release);
  lastKey = 0;
  keyActive = false;
  holdCounter = 0;
  keyboardLayout = std::make_unique<TerminalTextKeyboardLayout>();

  // Switch stdin to raw mode for immediate key reading
  tcgetattr(STDIN_FILENO, &origTermios);
  struct termios raw = origTermios;
  cfmakeraw(&raw);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void TerminalTextKB::scanKeyboard() {
  struct pollfd pfd;
  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;
  pfd.revents = 0;
  bool keyAvailable = (poll(&pfd, 1, 0) > 0);

  if (keyAvailable) {
    uint32_t key = readKey();
    if (key == 0) {
      return;
    }

    // skip repeats of the same key (still held)
    if (keyActive && key == lastKey) {
      holdCounter = 3;
      return;
    }

    // release previous key
    if (keyActive) {
      handleKeyEvent(lastKey, false, false, false, false);
    }

    // determine modifiers
    bool shift = false;
    bool ctrl = false;
    bool alt = false;

    if (key >= 0x01 && key <= 0x1A) {
      ctrl = true;
      key += 0x60;
    } else if (key >= 'A' && key <= 'Z') {
      shift = true;
      key += 0x20;
    }

    // press new key
    handleKeyEvent(key, shift, ctrl, alt, true);
    lastKey = key;
    keyActive = true;
    holdCounter = 3;
  } else if (keyActive) {
    if (holdCounter > 0) {
      holdCounter--;
    } else {
      // release only after holdCounter expires
      handleKeyEvent(lastKey, false, false, false, false);
      lastKey = 0;
      keyActive = false;
    }
  }
}

uint8_t TerminalTextKB::getKBCodeDC01() {
  return kbcode2.load(std::memory_order_acquire);
}
uint8_t TerminalTextKB::getKBCodeDC00() {
  return kbcode1.load(std::memory_order_acquire);
}
uint8_t TerminalTextKB::getShiftctrlcode() {
  return shiftctrlcode.load(std::memory_order_acquire);
}

uint8_t TerminalTextKB::getKBJoyValue() {
  uint8_t value = 0xff;
  if (kbKeyUp) {
    value &= ~(1 << 0);
  }
  if (kbKeyDown) {
    value &= ~(1 << 1);
  }
  if (kbKeyLeft) {
    value &= ~(1 << 2);
  }
  if (kbKeyRight) {
    value &= ~(1 << 3);
  }
  if (ctrlDown) {
    value &= ~(1 << 4);
  }
  return value;
}

#endif
