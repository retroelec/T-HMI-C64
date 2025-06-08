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
#ifndef KEYBOARDDRIVER_H
#define KEYBOARDDRIVER_H

#include <cstddef>
#include <cstdint>

class KeyboardDriver {
public:
  virtual void init() = 0;
  virtual uint8_t *getExtCmdBuffer() = 0;
  virtual void sendExtCmdNotification(uint8_t *data, size_t size) = 0;
  virtual void scanKeyboard() = 0;
  virtual uint8_t getDC01(uint8_t dc00, bool xchgports) = 0;
  virtual uint8_t getKBJoyValue(bool port2) = 0;
  virtual void setKBcodes(uint8_t sentdc01, uint8_t sentdc00) = 0;
  virtual void setDetectReleasekey(bool detectreleasekey) = 0;
  virtual ~KeyboardDriver() {}
};

#endif // KEYBOARDDRIVER_H
