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
#ifndef HARDWAREINITIALIZATIONEXCEPTION_H
#define HARDWAREINITIALIZATIONEXCEPTION_H

#include <stdexcept>
#include <string>

class HardwareInitializationException : public std::runtime_error {
public:
  HardwareInitializationException(const std::string &message)
      : std::runtime_error(message) {}
};

#endif // HARDWAREINITIALIZATIONEXCEPTION_H
