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
#ifndef JOYSTICKDRIVER_H
#define JOYSTICKDRIVER_H

#include <cstdint>

/**
 * @brief Interface for joystick drivers.
 *
 * This interface defines the methods required to implement a joystick driver
 * compatible with C64-style input, including directional input and fire
 * buttons.
 */
class JoystickDriver {
public:
  /**
   * @brief Initializes the joystick driver.
   *
   * This method should be called once during system startup to configure
   * any hardware or software resources required to read joystick input.
   */
  virtual void init() {};

  /**
   * @brief Returns the current state of the joystick directions and fire
   * button.
   *
   * The returned byte encodes the joystick status using the standard C64 bit
   * layout:
   * - bit 0: up
   * - bit 1: down
   * - bit 2: left
   * - bit 3: right
   * - bit 4: fire
   *
   * A cleared bit (0) means the direction/button is active (pressed),
   * and a set bit (1) means it is inactive (not pressed), following the C64
   * standard.
   *
   * @return uint8_t Bitfield representing the joystick state.
   */
  virtual uint8_t getValue() { return 0xff; }

  /**
   * @brief Returns the state of the optional second fire button.
   *
   * This method allows support for joysticks with a second fire button.
   *
   * @return true if the second fire button is pressed, false otherwise.
   */
  virtual bool getFire2() { return false; }

  virtual ~JoystickDriver() {}
};

#endif // JOYSTICKDRIVER_H
