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
#include "../platform/PlatformManager.h"
#include "SDLKeymap.h"
#include <SDL2/SDL.h>
#include <cstdint>

static const char *TAG = "SDLKB";

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
    }
    if (keyLeft) {
      joystickval &= ~(1 << 2);
    }
    if (keyDown) {
      joystickval &= ~(1 << 1);
    }
    if (keyUp) {
      joystickval &= ~(1 << 0);
    }
    if (keyFire) {
      joystickval &= ~(1 << 4);
    }
  }
  if (pressed) {
    if (mod & KMOD_LALT) {
      // help + quit
      if (key == SDLK_h) {
        PlatformManager::getInstance().log(LOG_INFO, TAG,
                                           "press alt-h for this help page");
        PlatformManager::getInstance().log(LOG_INFO, TAG,
                                           "press alt-q to quit the emulator");
        PlatformManager::getInstance().log(
            LOG_INFO, TAG,
            "press alt-l to load a program from directory %s, first type in "
            "the name of the "
            "program to load (without extension .prg)",
            Config::PATH);
        PlatformManager::getInstance().log(LOG_INFO, TAG,
                                           "press alt-r to reset the emulator");
        PlatformManager::getInstance().log(
            LOG_INFO, TAG,
            "press alt-j to switch between joystick in port 1, joystick in "
            "port 2, no joystick. Cursor keys and ctrl key are used as "
            "joystick keys if a joystick port is chosen");
      } else if (key == SDLK_q) {
        exit(0);
      }
      // "external command" keys
      else if (key == SDLK_l) {
        gotExternalCmd = true;
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::LOAD);
      } else if (key == SDLK_r) {
        gotExternalCmd = true;
        extCmdBuffer[0] =
            static_cast<std::underlying_type<ExtCmd>::type>(ExtCmd::RESET);
      } else if (key == SDLK_j) {
        switch (joystickmode) {
        case ExtCmd::KBJOYSTICKMODEOFF:
          joystickmode = ExtCmd::KBJOYSTICKMODE1;
          gotExternalCmd = true;
          extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
              ExtCmd::KBJOYSTICKMODE1);
          break;
        case ExtCmd::KBJOYSTICKMODE1:
          joystickmode = ExtCmd::KBJOYSTICKMODE2;
          gotExternalCmd = true;
          extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
              ExtCmd::KBJOYSTICKMODE2);
          break;
        case ExtCmd::KBJOYSTICKMODE2:
          joystickmode = ExtCmd::KBJOYSTICKMODEOFF;
          gotExternalCmd = true;
          extCmdBuffer[0] = static_cast<std::underlying_type<ExtCmd>::type>(
              ExtCmd::KBJOYSTICKMODEOFF);
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
      // C64 keys
      KeySpec k{key, (mod & KMOD_SHIFT), (mod & KMOD_CTRL)};
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

void SDLKB::scanKeyboard() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      exit(0);
    } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
      bool pressed = (event.type == SDL_KEYDOWN);
      handleKeyEvent(event.key.keysym.sym, SDL_GetModState(), pressed);
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

void SDLKB::sendExtCmdNotification(uint8_t *data, size_t size) {}

void SDLKB::setKBcodes(uint8_t sentdc01, uint8_t sentdc00) {
  kbcode2 = sentdc01;
  kbcode1 = sentdc00;
}

void SDLKB::setDetectReleasekey(bool detectreleasekey) {}
#endif
