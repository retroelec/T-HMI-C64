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
#ifndef FSDRIVER_H
#define FSDRIVER_H

#include <cstdint>

/**
 * @brief Interface for filesystem drivers.
 *
 * This interface defines methods for interacting with a filesystem. It provides
 * support for initialization, program loading and saving, as well as simple
 * directory iteration.
 */
class FSDriver {
public:
  /**
   * @brief Initializes access to the filesystem.
   *
   * This method sets up all necessary hardware and software components
   * required for filesystem access. It must be called before any other method.
   *
   * @return true if initialization succeeded, false otherwise.
   */
  virtual bool init() = 0;

  /**
   * @brief Loads a C64 program (.prg file) from the filesystem into RAM.
   *
   * Reads the specified file and loads it into emulated memory. The file is
   * expected to begin with a 2-byte little-endian load address (low byte
   * first), followed by the actual program data. The data is written starting
   * at that load address. The path to the file name is implementation specific.
   *
   * @param filename Null-terminated string specifying the file name.
   * @param ram Pointer to the base address of the emulated RAM.
   * @return uint16_t The end address of the loaded program in memory.
   */
  virtual uint16_t load(char *filename, uint8_t *ram) = 0;

  /**
   * @brief Saves a memory region to a C64-style program file (.prg).
   *
   * Writes a program from emulated RAM to the specified file. The file begins
   * with a 2-byte little-endian start address (low byte first), followed by the
   * memory content from start address to end address. The path to the file name
   * is implementation specific.
   *
   * @param filename Null-terminated string specifying the file name.
   * @param ram Pointer to the base address of the emulated RAM.
   * @param startaddr Start address of the memory region to be saved.
   * @param endaddr End address of the memory region to be saved.
   * @return true if the file was saved successfully, false otherwise.
   */
  virtual bool save(char *filename, uint8_t *ram, uint16_t startaddr,
                    uint16_t endaddr) = 0;

  /**
   * @brief Retrieves the next directory entry.
   *
   * Iterates over the entries in the current directory. The name of the next
   * file is copied into the provided buffer. If `start` is true, the iteration
   * starts from the first entry. If `start` is false, the iteration continues
   * from where it left off in the previous call.
   *
   * The buffer `nextentry` must be at least 17 bytes in size. The result will
   * be a null-terminated string (C-string) containing up to 16 characters of
   * the entry name.
   *
   * @param nextentry Pointer to the buffer where the next directory entry will
   * be stored.
   * @param start If true, (re)starts from the first entry; if false, continues
   * from last.
   * @return true if an entry was successfully written into `nextentry`,
   *         false if an error occurred.
   */
  virtual bool listnextentry(uint8_t *nextentry, bool start) = 0;

  virtual ~FSDriver() {}
};

#endif // FSDRIVER_H
