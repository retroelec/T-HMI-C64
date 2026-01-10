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
#ifndef SDLKB_H
#define SDLKB_H

#include "../Config.h"
#ifdef USE_SDL_KEYBOARD
#include "../ExtCmd.h"
#include "KeyboardDriver.h"
#include <SDL2/SDL.h>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <queue>

class SDLKB : public KeyboardDriver {
private:
  std::atomic<bool> gotExternalCmd = false;
  uint8_t extCmdBuffer[1024];

  bool joystickActive = false;
  ExtCmd joystickmode = ExtCmd::JOYSTICKMODEOFF;

  bool keyRight = false;
  bool keyLeft = false;
  bool keyUp = false;
  bool keyDown = false;
  bool keyFire = false;

  bool specialjoymode;

  bool attachwinopen = false;
  bool openattachwin = false;
  static const uint16_t DISKNAMEMAXLEN = 17;
  char diskname[DISKNAMEMAXLEN];
  SDL_Window *attachwin = NULL;
  SDL_Renderer *attachrenderer = NULL;

  std::atomic<uint8_t> kbcode1;
  std::atomic<uint8_t> kbcode2;
  std::atomic<uint8_t> shiftctrlcode;

  std::queue<SDL_Event> eventQueue;
  std::mutex eventMutex;
  std::mutex attachWinMutex;

  void setCodes(uint8_t code1, uint8_t code2, uint8_t ctrlcode);
  void handleKeyEvent(SDL_Keycode key, SDL_Keymod mod, bool pressed);
  void printHelpHint();

public:
  void init() override;
  uint8_t *getExtCmdData() override;
  void sendExtCmdNotification(uint8_t *data, size_t size) override;
  void syncAndCreateAttachWinSDL() override;
  void scanKeyboard() override;
  uint8_t getKBCodeDC01() override;
  uint8_t getKBCodeDC00() override;
  uint8_t getShiftctrlcode() override;
  uint8_t getKBJoyValue() override;
  void setDetectReleasekey(bool detectreleasekey) override;
  void setSpecialjoymode(bool specialjoymode) override;
  void setJoystickmode(ExtCmd joystickmode) override;
};
#endif

#endif // SDLKB_H
