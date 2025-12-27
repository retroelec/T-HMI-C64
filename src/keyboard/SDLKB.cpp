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

static const uint8_t CHARPIXSIZE = 4;
extern void drawChar(SDL_Renderer *ren, uint16_t c, uint16_t x, uint16_t y,
                     uint8_t size);

void SDLKB::setCodes(uint8_t code1, uint8_t code2, uint8_t ctrlcode) {
  kbcode1 = code1;
  kbcode2 = code2;
  shiftctrlcode = ctrlcode;
}

void SDLKB::handleKeyEvent(SDL_Keycode key, SDL_Keymod mod, bool pressed) {
  if (attachwinopen) {
    SDL_SetRenderDrawColor(attachrenderer, 0, 0, 50, 255);
    SDL_RenderClear(attachrenderer);
    uint16_t startX = 20;
    uint16_t y = 40;
    for (uint8_t i = 0; i < strlen(diskname); i++) {
      uint16_t ch = (unsigned char)diskname[i];
      if (ch >= 97 && ch <= 122) {
        ch += 160;
      }
      drawChar(attachrenderer, ch, startX + i * (8 * CHARPIXSIZE + 2), y,
               CHARPIXSIZE);
    }
    uint16_t cursorX =
        startX + (uint16_t)strlen(diskname) * (8 * CHARPIXSIZE + 2);
    SDL_SetRenderDrawColor(attachrenderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(attachrenderer, cursorX, y, cursorX,
                       y + 8 * CHARPIXSIZE);
    SDL_RenderPresent(attachrenderer);
    if (pressed) {
      if (key == SDLK_BACKSPACE) {
        size_t len = strlen(diskname);
        if (len > 0) {
          diskname[len - 1] = '\0';
        }
      } else if (key == SDLK_RETURN) {
        if (attachrenderer) {
          SDL_DestroyRenderer(attachrenderer);
        }
        if (attachwin) {
          SDL_DestroyWindow(attachwin);
        }
        attachwinopen = false;
        if (strlen(diskname) == 0) {
          extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
              ExtCmd::DETACHD64);
        } else {
          extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
              ExtCmd::ATTACHD64);
          std::copy(diskname, diskname + strlen(diskname) + 1,
                    extCmdBuffer + 3);
        }
        gotExternalCmd = true;
      } else {
        if (strlen(diskname) < DISKNAMEMAXLEN - 1) {
          uint8_t ch = (uint8_t)key;
          if ((ch >= 97 && ch <= 122) || (ch >= 48 && ch <= 57)) {
            size_t len = strlen(diskname);
            diskname[len] = key;
            diskname[len + 1] = '\0';
          }
        }
      }
    }
    return;
  }
  if (pressed) {
    if (mod & KMOD_LALT) {
      // help + quit
      if (key == SDLK_h) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::WRITETEXT);
        const uint8_t help[] = "\x93\x5\r"
                               "          **** HELP PAGE ****\r\r"
                               "ALT-H FOR THIS HELP PAGE\r"
                               "ALT-Q TO QUIT THE EMULATOR\r"
                               "ALT-L TO LOAD A PROGRAM\r"
                               "      (SEE CONFIG::PATH, README.MD)\r"
                               "ALT-S TO SAVE A PROGRAM\r"
                               "ALT-T TO LIST PROGRAMS\r"
                               "ALT-A TO ATTACH/DETACH A D64 FILE\r"
                               "ALT-R TO RESET THE EMULATOR\r"
                               "ALT-, TO DECREMENT SOUND VOLUME\r"
                               "ALT-. TO INCREMENT SOUND VOLUME\r"
                               "ALT-J TO SWITCH BETWEEN JOYSTICK\r"
                               "      IN PORT 1, JOYSTICK IN PORT 2,\r"
                               "      NO JOYSTICK. CURSOR AND CTRL\r"
                               "      KEYS ARE USED AS JOYSTICK KEYS\r"
                               "      IF A JOYSTICK PORT IS CHOSEN\r"
                               "ALT-N SHOW CONTENT OF CPU REGISTERS\r"
                               "ALT-D SWITCH TO DEBUG MODE AND BACK\r\x9a\0";
        memcpy(&extCmdBuffer[3], help, sizeof(help));
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
      } else if (key == SDLK_t) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::LIST);
        gotExternalCmd = true;
      } else if (key == SDLK_a) {
        if (!attachwinopen) {
          attachwin = SDL_CreateWindow("attach disk, enter d64 name",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED, 400, 200, 0);
          attachrenderer =
              SDL_CreateRenderer(attachwin, -1, SDL_RENDERER_ACCELERATED);
          attachwinopen = true;
          diskname[0] = '\0';
        }
      } else if (key == SDLK_n) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::SHOWREG);
        gotExternalCmd = true;
      } else if (key == SDLK_r) {
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::RESET);
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
        case ExtCmd::JOYSTICKMODEOFF:
          joystickmode = ExtCmd::JOYSTICKMODE1;
          extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
              ExtCmd::JOYSTICKMODE1);
          gotExternalCmd = true;
          break;
        case ExtCmd::JOYSTICKMODE1:
          joystickmode = ExtCmd::JOYSTICKMODE2;
          extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
              ExtCmd::JOYSTICKMODE2);
          gotExternalCmd = true;
          break;
        case ExtCmd::JOYSTICKMODE2:
          joystickmode = ExtCmd::JOYSTICKMODEOFF;
          extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
              ExtCmd::JOYSTICKMODEOFF);
          gotExternalCmd = true;
          break;
        default:
          break;
        }
        if (joystickmode != ExtCmd::JOYSTICKMODEOFF) {
          joystickActive = true;
        } else {
          joystickActive = false;
        }
      }
    } else {
      if (joystickActive || specialjoymode) {
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
      KeySpec k{key, mod & KMOD_SHIFT, mod & KMOD_CTRL, mod & KMOD_RALT};
      auto it = keyMap.find(k);
      if (it != keyMap.end()) {
        auto [b1, b2, b3] = it->second;
        setCodes(b1, b2, b3);
      } else if ((mod & KMOD_SHIFT) || (mod & KMOD_CTRL) || (mod & KMOD_RALT)) {
        // fallback: modifier pressed but no mapping found
        KeySpec k{key, false, false, false};
        auto it = keyMap.find(k);
        if (it != keyMap.end()) {
          auto [b1, b2, b3] = it->second;
          if (mod & KMOD_SHIFT) {
            b3 |= 1;
          } else if (mod & KMOD_LCTRL) {
            b3 |= 2;
          } else if (mod & KMOD_RALT) {
            b3 |= 4;
          }
          setCodes(b1, b2, b3);
        }
      }
    }
  } else {
    setCodes(0xff, 0xff, 0x00);
  }
}

void SDLKB::init() {
  SDL_InitSubSystem(SDL_INIT_EVENTS);
  specialjoymode = false;
}

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

uint8_t SDLKB::getKBJoyValue() { return 0xff; }

uint8_t *SDLKB::getExtCmdData() {
  if (gotExternalCmd) {
    gotExternalCmd = false;
    extCmdBuffer[2] = 0x80;
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

void SDLKB::setDetectReleasekey(bool detectreleasekey) {}

void SDLKB::setSpecialjoymode(bool specialjoymode) {
  this->specialjoymode = specialjoymode;
}

void SDLKB::setJoystickmode(ExtCmd joystickmode) {
  this->joystickmode = joystickmode;
  if (joystickmode != ExtCmd::JOYSTICKMODEOFF) {
    joystickActive = true;
  } else {
    joystickActive = false;
  }
}
#endif
