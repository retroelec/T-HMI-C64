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
   * @brief Performs a keyboard scan and updates internal variabels.
   *
   * Is called every 8ms.
   */
  virtual void scanKeyboard() = 0;

  /**
   * @brief Returns the current CIA1 port B ($DC01) or CIA1 port A ($DC00)
   * value.
   *
   * @param dc00 Value written to CIA1 port A ($DC00), selecting active columns.
   * @param xchgports If true, swaps port A/B handling to support port reversal.
   * @return Current value representing key states for the selected rows.
   */
  virtual uint8_t getDC01(uint8_t dc00, bool xchgports) = 0;

  /**
   * @brief Retrieves the current joystick emulation value.
   *
   * @return 8-bit value encoding joystick directions and fire button.
   */
  virtual uint8_t getKBJoyValue() = 0;

  /**
   * @brief Retrieves the data for an external command if one is requested.
   *
   * Is called once per frame.
   *
   * @return Pointer to the data containing the external command including
   * parameters or nullptr if no command is pending.
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

  virtual ~KeyboardDriver() {}
};

#endif // KEYBOARDDRIVER_H
