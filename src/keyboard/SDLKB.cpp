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
#ifdef USE_SDL_KEYBOARD
#include "../ExtCmd.h"
#include "../ExtCmdQueue.h"
#include "SDLKB.h"
#include "SDLKeymap.h"
#include "platform/PlatformManager.h"
#include <SDL2/SDL.h>
#include <cstdint>
#include <mutex>
#include <queue>

static const char *TAG = "SDLKB";

static const uint8_t CHARPIXSIZE = 4;
extern void drawChar(SDL_Renderer *ren, uint16_t c, uint16_t x, uint16_t y,
                     uint8_t size);

void SDLKB::setCodes(uint8_t code1, uint8_t code2, uint8_t ctrlcode) {
  kbcode1 = code1;
  kbcode2 = code2;
  shiftctrlcode = ctrlcode;
}

void SDLKB::handleKeyEvent(SDL_Keycode key, SDL_Keymod mod, bool pressed) {
  ExtCmdQueue::ExternalCmd extcmd;
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
        openattachwin = false;
        if (strlen(diskname) == 0) {
          extcmd.cmd = ExtCmd::DETACHD64;
        } else {
          extcmd.cmd = ExtCmd::ATTACHD64;
          std::copy(diskname, diskname + strlen(diskname) + 1,
                    &extcmd.param[3]);
        }
        ExtCmdQueue::getInstance().push(extcmd);
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
    if (mod & KMOD_RSHIFT) {
      // help + quit
      if (key == SDLK_h) {
        const uint8_t help[] = "\x93\x5\r"
                               "          **** HELP PAGE ****\r\r"
                               "RSHIFT-H FOR THIS HELP PAGE\r"
                               "RSHIFT-Q TO QUIT THE EMULATOR\r"
                               "RSHIFT-L TO LOAD A PROGRAM\r"
                               "         (SEE CONFIG::PATH, README.MD)\r"
                               "RSHIFT-S TO SAVE A PROGRAM\r"
                               "RSHIFT-T TO LIST PROGRAMS\r"
                               "RSHIFT-A TO ATTACH/DETACH A D64 FILE\r"
                               "RSHIFT-R TO RESET THE EMULATOR\r"
                               "RSHIFT-, TO DECREMENT SOUND VOLUME\r"
                               "RSHIFT-. TO INCREMENT SOUND VOLUME\r"
                               "RSHIFT-J TO SWITCH BETWEEN JOYSTICK\r"
                               "         IN PORT 1, JOYSTICK IN PORT 2,\r"
                               "         NO JOYSTICK. CURSOR AND CTRL\r"
                               "         KEYS ARE USED AS JOYSTICK KEYS\r"
                               "         IF A JOYSTICK PORT IS CHOSEN\r"
                               "RSHIFT-N SHOW CONTENT OF CPU REGISTERS\r"
                               "RSHIFT-D SWITCH TO DEBUG MODE AND BACK\r"
                               "RSHIFT-P TO PAUSE\r"
                               "COMMODORE KEY = LEFT ALT\r\x9a\0";
        extcmd.cmd = ExtCmd::WRITETEXT;
        int16_t helpsize = sizeof(help);
        uint8_t *helpptr = (uint8_t *)help;
        while (helpsize > 0) {
          memcpy(&extcmd.param[2], helpptr, (helpsize > 250) ? 250 : helpsize);
          ExtCmdQueue::getInstance().push(extcmd);
          helpsize -= 250;
          helpptr += 250;
        }
      } else if (key == SDLK_q) {
        exit(0);
      }
      // "external command" keys
      else if (key == SDLK_l) {
        extcmd.cmd = ExtCmd::LOAD;
        ExtCmdQueue::getInstance().push(extcmd);
      } else if (key == SDLK_s) {
        extcmd.cmd = ExtCmd::SAVE;
        ExtCmdQueue::getInstance().push(extcmd);
      } else if (key == SDLK_t) {
        extcmd.cmd = ExtCmd::LIST;
        ExtCmdQueue::getInstance().push(extcmd);
      } else if (key == SDLK_a) {
        if (!attachwinopen && !openattachwin) {
          std::lock_guard<std::mutex> lock(attachWinMutex);
          openattachwin = true;
          diskname[0] = '\0';
        }
      } else if (key == SDLK_n) {
        extcmd.cmd = ExtCmd::SHOWREG;
        ExtCmdQueue::getInstance().push(extcmd);
      } else if (key == SDLK_r) {
        extcmd.cmd = ExtCmd::RESET;
        ExtCmdQueue::getInstance().push(extcmd);
      } else if (key == SDLK_d) {
        extcmd.cmd = ExtCmd::SWITCHDEBUG;
        ExtCmdQueue::getInstance().push(extcmd);
      } else if (key == SDLK_COMMA) {
        extcmd.cmd = ExtCmd::DECVOLUME;
        extcmd.param[0] = 10;
        ExtCmdQueue::getInstance().push(extcmd);
      } else if (key == SDLK_PERIOD) {
        extcmd.cmd = ExtCmd::INCVOLUME;
        extcmd.param[0] = 10;
        ExtCmdQueue::getInstance().push(extcmd);
      } else if (key == SDLK_p) {
        extcmd.cmd = ExtCmd::PAUSE;
        ExtCmdQueue::getInstance().push(extcmd);
      } else if (key == SDLK_j) {
        switch (joystickmode) {
        case ExtCmd::JOYSTICKMODEOFF:
          joystickmode = ExtCmd::JOYSTICKMODE1;
          extcmd.cmd = ExtCmd::JOYSTICKMODE1;
          ExtCmdQueue::getInstance().push(extcmd);
          break;
        case ExtCmd::JOYSTICKMODE1:
          joystickmode = ExtCmd::JOYSTICKMODE2;
          extcmd.cmd = ExtCmd::JOYSTICKMODE2;
          ExtCmdQueue::getInstance().push(extcmd);
          break;
          break;
        case ExtCmd::JOYSTICKMODE2:
          joystickmode = ExtCmd::JOYSTICKMODEOFF;
          extcmd.cmd = ExtCmd::JOYSTICKMODEOFF;
          ExtCmdQueue::getInstance().push(extcmd);
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
      bool found = false;
      if (!((mod & KMOD_LCTRL) || (mod & KMOD_LALT))) {
        KeySpec k{key, mod & KMOD_LSHIFT, mod & KMOD_RALT};
        auto it = keyMap.find(k);
        if (it != keyMap.end()) {
          auto [b1, b2, b3] = it->second;
          setCodes(b1, b2, b3);
          found = true;
        }
      }
      if ((!found) &&
          ((mod & KMOD_LSHIFT) || (mod & KMOD_LCTRL) || (mod & KMOD_LALT))) {
        // fallback: no mapping found, but modifier (left) shift, (left) ctrl or
        // commodore (= left alt) pressed?
        KeySpec k{key, false, false};
        auto it = keyMap.find(k);
        if (it != keyMap.end()) {
          auto [b1, b2, b3] = it->second;
          if (mod & KMOD_LSHIFT) {
            b3 |= 1;
          } else if (mod & KMOD_LCTRL) {
            b3 |= 2;
          } else if (mod & KMOD_LALT) {
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

static uint8_t helpbox[] = "\x55\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43"
                           "\x43\x43\x43\x43\x43\x43\x43\x43\x49"
                           "\x42\x20\x12\x13\x08\x09\x06\x14\x2d\x08\x20\x06"
                           "\x0f\x12\x20\x08\x05\x0c\x10\x20\x42"
                           "\x4a\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43"
                           "\x43\x43\x43\x43\x43\x43\x43\x43\x4b";

void SDLKB::printHelpHint() {
  ExtCmdQueue::ExternalCmd extcmd;
  extcmd.cmd = ExtCmd::WRITEOSD;
  extcmd.param[2] = 9;
  extcmd.param[3] = 5;
  extcmd.param[4] = 21;
  extcmd.param[5] = 3;
  extcmd.param[6] = 1;
  extcmd.param[7] = 0;
  extcmd.param[8] = 5;
  extcmd.param[9] = 0;
  extcmd.param[10] = 1;
  memcpy(&extcmd.param[11], helpbox, 21 * 3);
  ExtCmdQueue::getInstance().push(extcmd);
}

void SDLKB::init() {
  SDL_InitSubSystem(SDL_INIT_EVENTS);
  specialjoymode = false;
  printHelpHint();
}

void SDLKB::syncAndCreateAttachWinSDL() {
  // synchronize SDL events with the main thread
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    std::lock_guard<std::mutex> lock(eventMutex);
    eventQueue.push(ev);
  }
  // create the attach window when needed
  if (openattachwin) {
    std::lock_guard<std::mutex> lock(attachWinMutex);
    openattachwin = false;
    attachwin =
        SDL_CreateWindow("attach disk, enter d64 name", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 400, 200, 0);
    if (!attachwin) {
      PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                         "error creating attach window");
      return;
    }
    attachrenderer =
        SDL_CreateRenderer(attachwin, -1, SDL_RENDERER_ACCELERATED);
    attachwinopen = true;
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
