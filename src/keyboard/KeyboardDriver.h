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

#include "../ExtCmd.h"
#include <cstddef>
#include <cstdint>

/**
 * @brief Interface for keyboard drivers.
 *
 * This interface defines the necessary functions to implement
 * keyboard input handling, external command processing,
 * and virtual joystick.
 */
class KeyboardDriver {
public:
  /**
   * @brief Initializes the keyboard driver.
   *
   * This method is called once during system startup to configure the keyboard.
   */
  virtual void init() = 0;

  /**
   * @brief Performs a keyboard scan and updates internal variables.
   *
   * This method is invoked periodically (every 8 ms) by an external process
   * or timer rather than the main execution flow. Because of this, all
   * involved variables that are read or modified within this method should
   * be declared as atomic to ensure safe concurrent access.
   */
  virtual void scanKeyboard() = 0;

  /**
   * @brief Retrieves the current keyboard code stored in the register dc01.
   *
   * @return 8-bit value of register dc01.
   */
  virtual uint8_t getKBCodeDC01() = 0;

  /**
   * @brief Retrieves the current keyboard code stored in the register dc00.
   *
   * @return 8-bit value of register dc00.
   */
  virtual uint8_t getKBCodeDC00() = 0;

  /**
   * @brief Retrieves the current "shift control code".
   *
   * shiftctrlcode: bit 0 -> shift
   *                bit 1 -> ctrl
   *                bit 2 -> commodore
   *                bit 7 -> external command
   *
   * @return 8-bit value of "shift control code".
   */
  virtual uint8_t getShiftctrlcode() = 0;

  /**
   * @brief Retrieves the current joystick emulation value.
   *
   * @return 8-bit value encoding joystick directions and fire button.
   */
  virtual uint8_t getKBJoyValue() = 0;

  /**
   * @brief Retrieves an external command including corresponding data.
   *
   * Is called once per frame.
   *
   * @return Pointer to the data containing the external command including
   * parameters. The null pointer is returned if no command is requested.
   * See enum class ExtCmd for the content of the data buffer.
   */
  virtual uint8_t *getExtCmdData() = 0;

  /**
   * @brief Sends an external command notification to the client.
   *
   * @param data Pointer to the data to send.
   * @param size Number of bytes to send.
   */
  virtual void sendExtCmdNotification(uint8_t *data, size_t size) = 0;

  /**
   * @brief Sets specific key codes for the keyboard matrix.
   *
   * @param sentdc01 Value for CIA1 port B.
   * @param sentdc00 Value for CIA1 port A.
   */
  virtual void setKBcodes(uint8_t sentdc01, uint8_t sentdc00) = 0;

  /**
   * @brief Enables or disables detection of key release events.
   *
   * @param detectreleasekey true to enable release detection, false to disable.
   */
  virtual void setDetectReleasekey(bool detectreleasekey) = 0;

  /**
   * @brief Polls keyboard events and enqueues them for later processing by the
   * method scanKeyboard(). The method is intended for SDL Keyboards only.
   *
   * Is called in the main thread (required on Windows) to keep input and window
   * handling responsive.
   */
  virtual void feedEvents() {}

  /**
   * @brief Sets the "gamemode" state.
   *
   * Used for systems which emulate joystick input via the keyboard.
   *
   * @return true if the button 3 is pressed, false otherwise.
   */
  virtual void setGamemode(bool gamemode) {}

  virtual void setJoystickmode(ExtCmd joystickmode) {}

  virtual ~KeyboardDriver() {}
};

#endif // KEYBOARDDRIVER_H
