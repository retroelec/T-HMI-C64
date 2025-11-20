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
#ifndef FLOPPY_H
#define FLOPPY_H

#include "fs/FileDriver.h"
#include "platform/PlatformManager.h"
#include <fstream>
#include <memory>
#include <string>
#include <vector>

class Floppy {
private:
  static constexpr uint8_t sectorsPerTrack[41] = {
      0,  21, 21, 21, 21, 21, 21, 21, 21, 21,
      21, 21, 21, 21, 21, 21, 21, 21,        // 1–17
      19, 19, 19, 19, 19, 19, 19,            // 18–24
      18, 18, 18, 18, 18, 18,                // 25–30
      17, 17, 17, 17, 17, 17, 17, 17, 17, 17 // 31–40
  };

  struct Channel {
    std::unique_ptr<FileDriver> file;
    uint8_t buffernr;
    bool hasChannelName;
    bool isOpen = false;
    uint16_t bufferidx;
    uint16_t buffersize;
  };

  static std::unique_ptr<FileDriver> sysfile;

  std::unique_ptr<FileDriver> d64file;
  std::vector<uint8_t> buffer[5];
  uint8_t errmessage[12];
  uint8_t errmessageidx;
  uint16_t freeBlocks;
  uint8_t track;
  uint8_t sector;
  uint8_t startTrack;
  uint8_t startSector;
  uint8_t diriterstate;
  uint16_t diriteraddr;
  Channel channels[16];
  std::string name;
  uint8_t device = 8;
  bool listening = false;
  bool talking = false;
  uint8_t currentSecondary = 0;
  bool collectName = false;
  bool triggererrorchannel = false;
  bool triggercmdchannel = false;

  int64_t calcOffset(uint8_t track, uint8_t sector) {
    int64_t off = 0;
    for (uint8_t t = 1; t < track; ++t) {
      off += static_cast<int64_t>(sectorsPerTrack[t]) * 256;
    }
    off += static_cast<int64_t>(sector) * 256;
    return off;
  }

  void initIterateDirectoryBlk() {
    track = buffer[4][0];
    sector = buffer[4][1];
  }

  // variables used outside this method:
  // startTrack, startSector, buf, track
  template <typename Callback>
  bool iterateDirectoryBlk(const std::string &filename,
                           std::vector<uint8_t> &buf, Callback cb) {
    if (track != 0) {
      if (!d64file->seek(calcOffset(track, sector), SEEK_SET)) {
        PlatformManager::getInstance().log(LOG_ERROR, "Floppy",
                                           "unsuccessful seek operation");
        return false;
      }
      if (d64file->read(buf.data(), 256) == 0) {
        PlatformManager::getInstance().log(LOG_ERROR, "Floppy",
                                           "unsuccessful read operation");
        return false;
      }
      track = buf[0];
      sector = buf[1];
      for (uint8_t e = 0; e < 8; e++) {
        uint8_t base = e * 32;
        uint8_t fileType = buf[base + 2];
        uint8_t fileTypeE = fileType & 0x0f;
        startTrack = buf[base + 3];
        startSector = buf[base + 4];
        std::string fname;
        for (uint8_t i = 0; i < 16; i++) {
          uint8_t b = buf[base + 5 + i];
          if (b == 160) {
            break;
          }
          fname.push_back(b);
        }
        if ((fileTypeE < 1) || (fileTypeE > 4)) {
          continue;
        }
        uint16_t blocks = buf[base + 30] + (buf[base + 31] << 8);
        if (cb(fname, filename, blocks, fileType)) {
          return true;
        }
      }
    }
    return false;
  }

  bool readNextDirBlk();
  bool readNextFileBlk();
  bool handleCmdChannel();
  bool directLoad();
  void initChannels();
  void initAttach();

public:
  static bool fsinitialized;

  bool d64attached = false;
  uint8_t lastStatus = 0;

  void init(uint8_t device);
  bool attach(const std::string &filename);
  void detach();
  uint8_t iecin();
  void iecout(uint8_t value);
  uint16_t load(const std::string &filename, uint8_t *ram);
  bool save(const std::string &filename, uint8_t *ram, uint16_t startaddr,
            uint16_t endaddr);
  bool listnextentry(uint8_t *nextentry, bool start);
};
#endif // FLOPPY_H
