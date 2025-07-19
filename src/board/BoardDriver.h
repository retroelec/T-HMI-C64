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
#ifndef BOARDDRIVER_H
#define BOARDDRIVER_H

#include <cstdint>

/**
 * @brief Interface for board-specific hardware control.
 *
 * This interface defines the essential methods that must be implemented
 * for hardware initialization, battery voltage monitoring, and power management
 * on a specific embedded board.
 */
class BoardDriver {
public:
  /**
   * @brief Initializes the board-specific hardware.
   *
   * This method must set up peripherals, pins, drivers, and any other
   * hardware-dependent components needed for the board to function.
   */
  virtual void init() = 0;

  /**
   * @brief Retrieves the current battery voltage in millivolts.
   *
   * The voltage is typically measured using an ADC pin and may include
   * calibration and compensation for a voltage divider if present.
   *
   * @return The battery voltage in millivolts.
   */
  virtual uint16_t getBatteryVoltage() = 0;

  /**
   * @brief Powers off the board.
   *
   * This method should perform all required steps to safely shut down
   * the board, such as disabling peripherals, saving state, and
   * initiating deep sleep or triggering hardware-controlled shutdown.
   */
  virtual void powerOff() = 0;

  virtual ~BoardDriver() {}
};

#endif // BOARDDRIVER_H
