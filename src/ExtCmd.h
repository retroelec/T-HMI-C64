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
#ifndef EXTCMD_H
#define EXTCMD_H

/**
 * @brief External commands for the emulator.
 *
 * External commands are sent to the emulator in a uint8_t buffer, see class
 * KeyboardDriver, method getExtCmdData. The first element (buf[0]) of the
 * buffer contains the command, i.e. a constant of the following enum. Some
 * commands need parameters which have to be placed in the following elements of
 * the buffer. The size of the buffer is implementation specfic.
 * Some commands (see description) send back a notification, see class
 * KeyboardDriver, method sendExtCmdNotification. The structure of the different
 * notifications is defined in NotificationStruct.h.
 */
enum class ExtCmd {
  /**
   * @brief No external command available.
   *
   * No parameters needed.
   */
  NOEXTCMD = 0,

  /**
   * @brief Joystick in port 1.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct1.
   */
  JOYSTICKMODE1 = 1,

  /**
   * @brief Joystick in port 2.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct1.
   */
  JOYSTICKMODE2 = 2,

  /**
   * @brief "Keyboard joystick" in port 1.
   *
   * No parameters needed.
   */
  KBJOYSTICKMODE1 = 3,

  /**
   * @brief "Keyboard joystick" in port 2.
   *
   * No parameters needed.
   */
  KBJOYSTICKMODE2 = 4,

  /**
   * @brief No joystick in any port.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct1.
   */
  JOYSTICKMODEOFF = 5,

  /**
   * @brief No "keyboard joystick" in any port.
   *
   * No parameters needed.
   */
  KBJOYSTICKMODEOFF = 6,

  /**
   * @brief Loads a program (prg file) stored in the configured directory to
   * memory.
   *
   * Name of the prg file is to the left of the actual cursor position (see
   * README.md). No parameters needed.
   */
  LOAD = 11,

  /**
   * @brief Receives a program using consecutive calls of
   * KeyboardDriver.getExtCmdData() and put it to memory.
   *
   * A simple protocol is used to receive the whole program in blocks of 250
   * bytes (in buffer):
   * - byte 0: cmd
   * - byte 1: cmd detail: first block (1), next block (0), last block (2)
   * - byte 2: cmd flag (unused)
   * - first block: byte 3 - 4: start address, 5 - 252: data
   * - next block: byte 3 - 252: data
   * - last block: byte 3: length of last block, byte 4 - (length+4-1): data
   */
  RECEIVEDATA = 12,

  /**
   * @brief Gets content of CPU registers and "important" VIC and CIA registers.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct2.
   */
  SHOWREG = 13,

  /**
   * @brief Gets the content of 16 bytes memory starting from a given address.
   *
   * Start address of memory is transfered in byte 3 + 4 of the buffer.
   * This command sends back a notification of type NotificationStruct3.
   */
  SHOWMEM = 14,

  /**
   * @brief Sends pressing the RESTORE key or the combination RESTORE +
   * RUN/STOP.
   *
   * If the second element of the buffer (buffer[1]) equals to 1 the combination
   * RESTORE + RUN/STOP is sent, only RESTORE otherwise.
   */
  RESTORE = 15,

  /**
   * @brief Resets the emulator.
   *
   * No parameters needed.
   */
  RESET = 20,

  /**
   * @brief Gets the state of the emulator.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct1.
   */
  GETSTATUS = 21,

  /**
   * @brief Switches the "deaktivate temp" switch (used for "emulator hacks"
   * like "ignoring badlines").
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct1.
   */
  SWITCHDEACTIVATETEMP = 22,

  /**
   * @brief Switches to "send raw key codes mode" and back.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct1.
   */
  SENDRAWKEYS = 24,

  /**
   * @brief Switches to "debug mode" and back.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct1.
   */
  SWITCHDEBUG = 25,

  /**
   * @brief Switches to "show performance mode" and back.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct1.
   */
  SWITCHPERF = 26,

  /**
   * @brief Switches to "detect releasing of a key mode" and back.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct1.
   */
  SWITCHDETECTRELEASEKEY = 27,

  /**
   * @brief Gets the voltage of the battery.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct5.
   */
  GETBATTERYVOLTAGE = 29,

  /**
   * @brief Powers off the device.
   *
   * No parameters needed.
   */
  POWEROFF = 30,

  /**
   * @brief Saves a program (prg file) from memory to a file in the configured
   * directory.
   *
   * Name of the prg file is to the left of the actual cursor position (see
   * README.md). No parameters needed.
   */
  SAVE = 31,

  /**
   * @brief Lists files stored in the configured directory.
   *
   * Consecutive calls of this command are needed to show all files, if more
   * than 23 files are stored in the configured directory. No parameters needed.
   */
  LIST = 32,

  /**
   * @brief Sets the sound volume of the emulator.
   *
   * The second element of the buffer (buffer[1]) contains the sound volume
   * (0-255).
   */
  SETVOLUME = 33,

  /**
   * @brief Increases the sound volume of the emulator.
   *
   * The second element of the buffer (buffer[1]) contains the increment.
   */
  INCVOLUME = 34,

  /**
   * @brief Decreases the sound volume of the emulator.
   *
   * The second element of the buffer (buffer[1]) contains the decrement.
   */
  DECVOLUME = 35,

  /**
   * @brief Writes a text to the C64 text screen (only if in "C64 text mode").
   *
   * The text to be displayed is stored starting at buffer position 4 (from
   * buffer[3]).
   */
  WRITETEXT = 36,

  /**
   * @brief Attach a d64 file to the floppy drive emulation.
   *
   * The name of the d64 file is stored starting at buffer position 4 (from
   * buffer[3]).
   * This command sends back a notification of type NotificationStruct1.
   */
  ATTACHD64 = 37,

  /**
   * @brief Detach a d64 file from the floppy drive emulation.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct1.
   */
  DETACHD64 = 38,

  /**
   * @brief Writes an OSD to the C64 screen.
   *
   * Parameters:
   * - byte 3: start x position
   * - byte 4: start y position
   * - byte 5: width
   * - byte 6: height
   * - byte 7: text color
   * - byte 8: background color
   * - byte 9/10: duration of the display
   * - byte 11: index of overlay (0 or 1)
   * - byte 12-: text to be displayed
   */
  WRITEOSD = 39,
  PAUSE = 40
};

#endif // EXTCMD_H
