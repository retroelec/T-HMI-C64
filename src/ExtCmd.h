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
#ifndef EXTCMD_H
#define EXTCMD_H

/**
 * @brief External commands for the emulator.
 *
 * External commands are sent to the emulator in a uint8_t buffer. The first
 * element (buf[0]) of the buffer contains the command, i.e. a constant of the
 * following enum. Some commands need parameters which have to be placed in the
 * following elements of the buffer. The size of the buffer is implementation
 * specfic. Some commands (see description) send back a notification, see class
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
   */
  JOYSTICKMODE1 = 1,

  /**
   * @brief Joystick in port 2.
   *
   * No parameters needed.
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
   * @brief Receives a program by gettimg chunks of data and put it to memory.
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
   * @brief Sends pressing the RESTORE key or the combination RESTORE +
   * RUN/STOP.
   *
   * If the param[0] equals to 1 the combination RESTORE + RUN/STOP is sent,
   * only RESTORE otherwise.
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
   * @brief deprecated
   *
   * No parameters needed.
   */
  SWITCHDEACTIVATETEMP = 22,

  /**
   * @brief Switches to "send raw key codes mode" and back.
   *
   * No parameters needed.
   */
  SENDRAWKEYS = 24,

  /**
   * @brief Switches to "debug mode" and back.
   *
   * No parameters needed.
   */
  SWITCHDEBUG = 25,

  /**
   * @brief Switches to "show performance mode" and back.
   *
   * No parameters needed.
   */
  SWITCHPERF = 26,

  /**
   * @brief Switches to "detect releasing of a key mode" and back.
   *
   * No parameters needed.
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
   * The first parameter (param[0]) contains the sound volume (0-255).
   */
  SETVOLUME = 33,

  /**
   * @brief Increases the sound volume of the emulator.
   *
   * The first parameter (param[0]) contains the increment.
   */
  INCVOLUME = 34,

  /**
   * @brief Decreases the sound volume of the emulator.
   *
   * The first parameter (param[0]) contains the decrement.
   */
  DECVOLUME = 35,

  /**
   * @brief Writes a text to the C64 text screen (only if in "C64 text mode").
   *
   * The text to be displayed is stored starting at buffer position 4 resp.
   * &param[2].
   */
  WRITETEXT = 36,

  /**
   * @brief Attach a d64 file to the floppy drive emulation.
   *
   * The name of the d64 file is stored starting at buffer position 4 resp.
   * &param[2].
   */
  ATTACHD64 = 37,

  /**
   * @brief Detach a d64 file from the floppy drive emulation.
   *
   * No parameters needed.
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

  /**
   * @brief Pausees the emulation.
   *
   * No parameters needed.
   */
  PAUSE = 40,

  /**
   * @brief Special command which waits for the number of steps given until the
   * next command in the queue is executed.
   *
   * The first parameter (param[0]) contains the number of steps (1 step = 20
   * ms) to wait for.
   */
  WAIT = 41,

  /**
   * @brief Loads and starts the given program.
   *
   * The name of the program to be loaded (prg file without extension ".prg")
   * starting at buffer position 4 resp. &param[2].
   */
  AUTOSTART = 42,

  /**
   * @brief Calls the display specific (optional) method setSpecial1().
   *
   * No parameters needed.
   */
  SPECIAL1 = 43,

  /**
   * @brief Calls the display specific (optional) method setSpecial2().
   *
   * No parameters needed.
   */
  SPECIAL2 = 44,

  /**
   * @brief Simulates a key press on the C64 keyboard.
   *
   * Parameters:
   * - param[0]: dc00 value (matrix column)
   * - param[1]: dc01 value (matrix row)
   * - param[2]: shiftctrlcode (bit 0=shift, bit 1=ctrl, bit 2=commodore)
   * - param[3]: duration (number of scans, 1 scan = 8ms, 0 = use default)
   */
  SENDKEY = 45,

  /**
   * @brief Sets the joystick value via REST API.
   *
   * Parameters:
   * - param[0]: joystick bitfield (like getValue(): bit 0=up, 1=down, 2=left,
   *             3=right, 4=fire1, 0xff = released)
   */
  SENDJOY = 46,

  /**
   * @brief Saves a snapshot (RAM, CPU, VIC, CIA, SID) to the internal buffer.
   *
   * No parameters needed.
   */
  SAVESNAPSHOT = 47,

  /**
   * @brief Restores emulator state from the previously saved snapshot.
   *
   * No parameters needed.
   */
  LOADSNAPSHOT = 48,

  /**
   * @brief Gets the THMIC64KB version of the emulator.
   *
   * No parameters needed.
   * This command sends back a notification of type NotificationStruct6.
   */
  GETVERSION = 49,
};

#endif // EXTCMD_H
