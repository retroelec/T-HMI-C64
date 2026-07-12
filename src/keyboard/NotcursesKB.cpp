/*
  Copyright (C) 2024-2026 retroelec <retroelec42@gmail.com>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 3 of the License, or (at your
  option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  For the complete text of the GNU General Public License see
  http://www.gnu.org/licenses/.
*/
#include "../Config.h"
#ifdef USE_NOTCURSES_KEYBOARD
#include "../ExtCmd.h"
#include "../ExtCmdQueue.h"
#include "../FileConfig.h"
#include "../platform/PlatformManager.h"
#include "NotcursesKB.h"
#include "NotcursesKeyboardEvent.h"
#include "NotcursesKeyboardFactory.h"
#include "display/NotcursesDisplay.h"
#include <cstdint>
#include <cstring>
#include <notcurses/notcurses.h>
#include <optional>
#include <poll.h>

NotcursesKB::NotcursesKB() : kbcode1(0xff), kbcode2(0xff), shiftctrlcode(0) {}
NotcursesKB::~NotcursesKB() = default;

void NotcursesKB::setCodes(uint8_t code1, uint8_t code2, uint8_t ctrlcode) {
  kbcode1 = code1;
  kbcode2 = code2;
  shiftctrlcode = ctrlcode;
}

std::optional<CodeTripleS> NotcursesKB::getKeyCodes(uint32_t key, bool shift,
                                                    bool alt) {
  auto &map = keyboardLayout->getMapping();
  KeySpec spec{key, shift, alt};
  auto it = map.find(spec);
  if (it != map.end()) {
    return it->second;
  }
  return std::nullopt;
}

void NotcursesKB::handleKeyEvent(uint32_t ch, bool shift, bool ctrl, bool alt,
                                 bool pressed) {
  ctrlDown = ctrl;

  uint32_t key = keyboardLayout->normalizeKey(ch);
  if (key != ch && !alt) {
    shift = true;
  }
  if (ch >= 'A' && ch <= 'Z') {
    shift = true;
  }

  if (joystickActive) {
    switch (ch) {
    case NCKEY_UP:
      kbKeyUp = pressed;
      return;
    case NCKEY_DOWN:
      kbKeyDown = pressed;
      return;
    case NCKEY_LEFT:
      kbKeyLeft = pressed;
      return;
    case NCKEY_RIGHT:
      kbKeyRight = pressed;
      return;
    }
  }

  if (pressed && ctrl) {
    ExtCmdQueue::ExternalCmd extcmd;
    switch (ch) {
    case 'h':
    case 'H': {
      static const uint8_t help[] = "\x93\x5\r"
                                    "          **** HELP PAGE ****\r\r"
                                    "CTRL-H FOR THIS HELP PAGE\r"
                                    "CTRL-Q TO QUIT THE EMULATOR\r"
                                    "CTRL-L TO LOAD A PROGRAM\r"
                                    "         (SEE CONFIG::PATH, README.MD)\r"
                                    "CTRL-S TO SAVE A PROGRAM\r"
                                    "CTRL-T TO LIST PROGRAMS\r"
                                    "CTRL-A TO ATTACH/DETACH A D64 FILE\r"
                                    "CTRL-R TO RESET THE EMULATOR\r"
                                    "CTRL-, TO DECREMENT SOUND VOLUME\r"
                                    "CTRL-. TO INCREMENT SOUND VOLUME\r"
                                    "CTRL-K TO SWITCH BETWEEN KB-JOYSTICK\r"
                                    "        IN PORT 1, IN PORT 2, NO\r"
                                    "        JOYSTICK. CURSOR AND CTRL KEYS\r"
                                    "        ARE USED AS JOYSTICK KEYS\r"
                                    "CTRL-J TO SWITCH BETWEEN JOYSTICK\r"
                                    "        IN PORT 1, IN PORT 2, NO\r"
                                    "        JOYSTICK\r"
                                    "CTRL-P TO PAUSE\r"
                                    "COMMODORE KEY = LEFT ALT\r\x9a\0";
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
    case 'a':
    case 'A':
      extcmd.cmd = ExtCmd::ATTACHD64;
      break;
    case 'n':
    case 'N':
      extcmd.cmd = ExtCmd::SHOWREG;
      break;
    case 'd':
    case 'D':
      extcmd.cmd = ExtCmd::SWITCHDEBUG;
      break;
    case ',':
      extcmd.cmd = ExtCmd::DECVOLUME;
      break;
    case '.':
      extcmd.cmd = ExtCmd::INCVOLUME;
      break;
    case 'p':
    case 'P':
      extcmd.cmd = ExtCmd::PAUSE;
      break;
    case 'j':
    case 'J':
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
    case 'k':
    case 'K':
      switch (kbjoystickmode) {
      case ExtCmd::KBJOYSTICKMODEOFF:
        kbjoystickmode = ExtCmd::KBJOYSTICKMODE1;
        extcmd.cmd = ExtCmd::KBJOYSTICKMODE1;
        break;
      case ExtCmd::KBJOYSTICKMODE1:
        kbjoystickmode = ExtCmd::KBJOYSTICKMODE2;
        extcmd.cmd = ExtCmd::KBJOYSTICKMODE2;
        break;
      case ExtCmd::KBJOYSTICKMODE2:
        kbjoystickmode = ExtCmd::KBJOYSTICKMODEOFF;
        extcmd.cmd = ExtCmd::KBJOYSTICKMODEOFF;
        break;
      default:
        break;
      }
      joystickActive = (kbjoystickmode != ExtCmd::KBJOYSTICKMODEOFF);
      ExtCmdQueue::getInstance().push(extcmd);
      return;
    default:
      return;
    }
    ExtCmdQueue::getInstance().push(extcmd);
    return;
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

void NotcursesKB::init() {
  kbcode1 = 0xff;
  kbcode2 = 0xff;
  shiftctrlcode = 0;
  std::string sdlkeyboardlayout = FileConfig::getSdlKeyboardLayout();
  keyboardLayout =
      NotcursesKeyboardFactory::createFromString(sdlkeyboardlayout);
}

void NotcursesKB::scanKeyboard() {
  if (!NotcursesDisplay::instance || !NotcursesDisplay::instance->nc) {
    return;
  }

  struct notcurses *nc = NotcursesDisplay::instance->nc;
  int fd = notcurses_inputready_fd(nc);
  if (fd < 0) {
    return;
  }

  struct pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN;
  pfd.revents = 0;
  if (poll(&pfd, 1, 0) <= 0) {
    return;
  }

  struct ncinput ni;
  while (notcurses_get_nblock(nc, &ni) > 0) {
    bool shift = (ni.modifiers & NCKEY_MOD_SHIFT) != 0;
    bool ctrl = (ni.modifiers & NCKEY_MOD_CTRL) != 0;
    bool alt = (ni.modifiers & NCKEY_MOD_ALT) != 0;
    handleKeyEvent(ni.id, shift, ctrl, alt,
                   ni.evtype == NCTYPE_PRESS || ni.evtype == NCTYPE_REPEAT);
  }
}

uint8_t NotcursesKB::getKBCodeDC01() { return kbcode2; }
uint8_t NotcursesKB::getKBCodeDC00() { return kbcode1; }
uint8_t NotcursesKB::getShiftctrlcode() { return shiftctrlcode; }

uint8_t NotcursesKB::getKBJoyValue() {
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