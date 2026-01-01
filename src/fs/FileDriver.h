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
#ifndef FILEDRIVER_H
#define FILEDRIVER_H

#include <cstdint>
#include <string>

/**
 * @brief Interface for basic file operations.
 *
 * This interface defines a generic, platform-independent API for performing
 * basic file I/O operations and managing access to the underlying filesystem.
 *
 * Implementations must provide an @ref init() method to initialize any required
 * hardware or software components before file access is possible (e.g., SD card
 * setup on embedded platforms). This method should be safe to call multiple
 * times and must complete successfully before any file operations are
 * performed.
 *
 * Implementations are expected to maintain an internal file instance that
 * represents the currently opened file. The instance should be created and
 * initialized by the @ref open() method, and used by subsequent operations
 * such as @ref read(), @ref write(), and @ref close().
 *
 * Each implementation is responsible for proper resource management, ensuring
 * that file handles are correctly initialized, reused, and released as needed.
 */

#include <string>

class FileDriver {
public:
  /**
   * @brief Initializes access to the filesystem.
   *
   * This method sets up all necessary hardware and software components
   * required for filesystem access. It must be called before any other
   * file operations are performed.
   *
   * Implementations are responsible for ensuring that initialization
   * is performed only once, if required. Multiple calls to this method
   * should have no adverse effects or should be safely ignored after
   * successful initialization.
   *
   * @return true if initialization succeeded, false otherwise.
   */
  virtual bool init() { return true; }

  /**
   * @brief Opens a file with the specified path and mode.
   *
   * This method attempts to open a file using the given file path and access
   * mode. Common modes include "r", "w", "rb", "wb", etc.
   *
   * @param path The path of the file to open.
   * @param mode The file access mode (e.g., "r", "w", "rb", "wb").
   * @return true if the file was successfully opened, false otherwise.
   */
  virtual bool open(const std::string &path, const char *mode) = 0;

  /**
   * @brief Reads a number of bytes from the file into a buffer.
   *
   * Reads up to @p count bytes from the currently opened file into the provided
   * buffer.
   *
   * @param buffer Pointer to the buffer where the read data will be stored.
   * @param count Number of bytes to read.
   * @return The number of bytes actually read.
   */
  virtual size_t read(void *buffer, size_t count) = 0;

  /**
   * @brief Writes a number of bytes from a buffer to the file.
   *
   * Writes up to @p count bytes from the provided buffer to the currently
   * opened file.
   *
   * @param buffer Pointer to the data buffer containing bytes to write.
   * @param count Number of bytes to write.
   * @return The number of bytes actually written.
   */
  virtual size_t write(const void *buffer, size_t count) = 0;

  /**
   * @brief Moves the file position indicator to a new location.
   *
   * Repositions the internal file pointer relative to the specified origin.
   *
   * @param offset The number of bytes to move relative to @p origin.
   * @param origin The reference position. Typically one of:
   *  - SEEK_SET (beginning of file)
   *  - SEEK_CUR (current position)
   *  - SEEK_END (end of file)
   * @return true if the operation was successful, false otherwise.
   */
  virtual bool seek(long offset, int origin) = 0;

  /**
   * @brief Returns the current file position.
   *
   * @return The current position in the file, in bytes from the beginning.
   */
  virtual long tell() const = 0;

  /**
   * @brief Checks whether the end of the file has been reached.
   *
   * @return true if the end of the file has been reached, false otherwise.
   */
  virtual bool eof() = 0;

  /**
   * @brief Returns the total size of the file.
   *
   * @return The file size in bytes.
   */
  virtual int64_t size() = 0;

  /**
   * @brief Closes the currently opened file.
   *
   * Releases any resources associated with the file handle.
   * Must not crash if called on an unopened file.
   */
  virtual void close() = 0;

  /**
   * @brief Retrieves the next directory entry.
   *
   * Iterates over the entries in the current directory. The name of the next
   * file is returned. If `start` is true, the iteration starts from the first
   * entry. If `start` is false, the iteration continues from where it left off
   * in the previous call.
   *
   * @param start If true, (re)starts from the first entry; if false, continues
   * from last.
   * @param name String where the next directory entry will be stored. Must be
   * empty if no more entries are left.
   * @return true if an entry was successfully written into `nextentry`,
   *         false if an error occurred.
   */
  virtual bool listnextentry(std::string &name, bool start) { return false; }

  virtual ~FileDriver() = default;
};

#endif // FILEDRIVER_H
