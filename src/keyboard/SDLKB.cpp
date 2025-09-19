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
#include "SDLKB.h"
#include "../Config.h"
#ifdef USE_SDL_KEYBOARD
#include "../ExtCmd.h"
#include "SDLKeymap.h"
#include <SDL2/SDL.h>
#include <cstdint>
#include <mutex>
#include <queue>

void SDLKB::char2codes(uint8_t code1, uint8_t code2, uint8_t ctrlcode) {
  kbcode1 = code1;
  kbcode2 = code2;
  shiftctrlcode = ctrlcode;
}

void SDLKB::handleKeyEvent(SDL_Keycode key, SDL_Keymod mod, bool pressed) {
  if (joystickActive) {
    // joystick emulation
    switch (key) {
    case SDLK_RIGHT:
      keyRight = pressed;
      break;
    case SDLK_LEFT:
      keyLeft = pressed;
      break;
    case SDLK_DOWN:
      keyDown = pressed;
      break;
    case SDLK_UP:
      keyUp = pressed;
      break;
    case SDLK_LCTRL:
      keyFire = pressed;
      break;
    }
    joystickval = 0xff;
    if (keyRight) {
      joystickval &= ~(1 << 3);
    } else if (keyLeft) {
      joystickval &= ~(1 << 2);
    }
    if (keyDown) {
      joystickval &= ~(1 << 1);
    } else if (keyUp) {
      joystickval &= ~(1 << 0);
    }
    if (keyFire) {
      joystickval &= ~(1 << 4);
    }
  }
  if (key == SDLK_TAB) {
    if (pressed) {
      commodoreKeyPressed = true;
    } else {
      commodoreKeyPressed = false;
    }
  }
  if (pressed) {
    if (mod & KMOD_LALT) {
      // help + quit
      if (key == SDLK_h) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::WRITETEXT);
        const uint8_t help[] = "\x93\r"
                               "          **** HELP PAGE ****\r\r"
                               "ALT-H FOR THIS HELP PAGE\r"
                               "ALT-Q TO QUIT THE EMULATOR\r"
                               "ALT-L TO LOAD A PROGRAM\r"
                               "      (SEE CONFIG::PATH, README.MD)\r"
                               "ALT-S TO SAVE A PROGRAM\r"
                               "ALT-R TO RESET THE EMULATOR\r"
                               "ALT-T FOR KEY COMB. RUN/STOP - RESTORE\r"
                               "ALT-Z FOR KEY RESTORE\r"
                               "ALT-, TO DECREMENT SOUND VOLUME\r"
                               "ALT-. TO INCREMENT SOUND VOLUME\r"
                               "ALT-J TO SWITCH BETWEEN JOYSTICK\r"
                               "      IN PORT 1, JOYSTICK IN PORT 2,\r"
                               "      NO JOYSTICK. CURSOR KEYS AND\r"
                               "      CTRL ARE USED AS JOYSTICK KEYS\r"
                               "      IF A JOYSTICK PORT IS CHOSEN\r"
                               "ALT-N SHOW CONTENT OF CPU REGISTERS\r"
                               "ALT-D SWITCH TO DEBUG MODE AND BACK\r\0";
        memcpy(&extCmdBuffer[1], help, sizeof(help));
        gotExternalCmd = true;
      } else if (key == SDLK_q) {
        exit(0);
      }
      // "external command" keys
      else if (key == SDLK_l) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::LOAD);
        gotExternalCmd = true;
      } else if (key == SDLK_s) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::SAVE);
        gotExternalCmd = true;
      } else if (key == SDLK_n) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::SHOWREG);
        gotExternalCmd = true;
      } else if (key == SDLK_r) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::RESET);
        gotExternalCmd = true;
      } else if (key == SDLK_t) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::RESTORE);
        extCmdBuffer[1] = 0x01;
        gotExternalCmd = true;
      } else if (key == SDLK_z) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::RESTORE);
        extCmdBuffer[1] = 0x00;
        gotExternalCmd = true;
      } else if (key == SDLK_d) {
        extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
            ExtCmd::SWITCHDEBUG);
        gotExternalCmd = true;
      } else if (key == SDLK_COMMA) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::DECVOLUME);
        extCmdBuffer[1] = 10;
        gotExternalCmd = true;
      } else if (key == SDLK_PERIOD) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::INCVOLUME);
        extCmdBuffer[1] = 10;
        gotExternalCmd = true;
      } else if (key == SDLK_j) {
        switch (joystickmode) {
        case ExtCmd::KBJOYSTICKMODEOFF:
          joystickmode = ExtCmd::KBJOYSTICKMODE1;
          extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
              ExtCmd::KBJOYSTICKMODE1);
          gotExternalCmd = true;
          break;
        case ExtCmd::KBJOYSTICKMODE1:
          joystickmode = ExtCmd::KBJOYSTICKMODE2;
          extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
              ExtCmd::KBJOYSTICKMODE2);
          gotExternalCmd = true;
          break;
        case ExtCmd::KBJOYSTICKMODE2:
          joystickmode = ExtCmd::KBJOYSTICKMODEOFF;
          extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
              ExtCmd::KBJOYSTICKMODEOFF);
          gotExternalCmd = true;
          break;
        default:
          break;
        }
        if (joystickmode != ExtCmd::KBJOYSTICKMODEOFF) {
          joystickActive = true;
        } else {
          joystickActive = false;
        }
      }
    } else {
      if (joystickActive) {
        switch (key) {
        case SDLK_RIGHT:
        case SDLK_LEFT:
        case SDLK_DOWN:
        case SDLK_UP:
        case SDLK_LCTRL:
          return;
        }
      }
      // C64 keys
      KeySpec k{key, (mod & KMOD_SHIFT), (mod & KMOD_CTRL),
                commodoreKeyPressed};
      auto it = keyMap.find(k);
      if (it != keyMap.end()) {
        auto [b1, b2, b3] = it->second;
        char2codes(b1, b2, b3);
      }
    }
  } else {
    char2codes(0xff, 0xff, 0x00);
  }
}

void SDLKB::init() { SDL_InitSubSystem(SDL_INIT_EVENTS); }

void SDLKB::feedEvents() {
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    std::lock_guard<std::mutex> lock(eventMutex);
    eventQueue.push(ev);
  }
}

void SDLKB::scanKeyboard() {
  std::lock_guard<std::mutex> lock(eventMutex);
  while (!eventQueue.empty()) {
    auto ev = eventQueue.front();
    eventQueue.pop();
    if (ev.type == SDL_QUIT) {
      exit(0);
    } else if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
      handleKeyEvent(ev.key.keysym.sym, SDL_GetModState(),
                     ev.type == SDL_KEYDOWN);
    }
  }
}

uint8_t SDLKB::getKBCodeDC01() { return kbcode2; }

uint8_t SDLKB::getKBCodeDC00() { return kbcode1; }

uint8_t SDLKB::getShiftctrlcode() { return shiftctrlcode; }

uint8_t SDLKB::getKBJoyValue() { return joystickval; }

uint8_t *SDLKB::getExtCmdData() {
  if (gotExternalCmd) {
    gotExternalCmd = false;
    return extCmdBuffer;
  }
  return nullptr;
}

void SDLKB::sendExtCmdNotification(uint8_t *data, size_t size) {
  // for (uint8_t i = 0; i < size; i++) {
  //   PlatformManager::getInstance().log(LOG_INFO, TAG, "notification byte %d:
  //   %d", i, data[i]);
  // }
}

void SDLKB::setKBcodes(uint8_t sentdc01, uint8_t sentdc00) {
  kbcode2 = sentdc01;
  kbcode1 = sentdc00;
}

void SDLKB::setDetectReleasekey(bool detectreleasekey) {}
#endif
