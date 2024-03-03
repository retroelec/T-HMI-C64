/*
 Copyright (C) 2024 retroelec <retroelec42@gmail.com>

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
#ifndef BLEKB_H
#define BLEKB_H

#include <stdint.h>
#include <string>

class BLEKB {
public:
  void init(std::string service_uuid, std::string characteristic_uuid,
            uint8_t *kbbuffer);
  // keyboard
  uint8_t getKBCode();
  uint8_t decode(uint8_t dc00);
  // transfer data
  bool getData(uint8_t *data);
};

#endif // BLEKB_H
