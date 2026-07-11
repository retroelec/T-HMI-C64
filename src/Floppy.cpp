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
#include "Floppy.h"
#include "Config.h"
#include "fs/FileFactory.h"
#include "platform/PlatformManager.h"
#include "roms/1541.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

static const char *TAG = "Floppy";

std::string decodeFileType(uint8_t t) {
  int f = t & 0x0f;
  bool closed = (t & 0x80) != 0;
  bool locked = (t & 0x40) != 0;
  std::string n;
  switch (f) {
  case 1:
    n = "SEQ";
    break;
  case 2:
    n = "PRG";
    break;
  case 3:
    n = "USR";
    break;
  case 4:
    n = "REL";
    break;
  default:
    n = "???";
    break;
  }
  if (!closed)
    n = n + "*";
  if (locked)
    n = n + ">";
  return n;
}

uint16_t countBitsLimited(uint32_t mask, uint8_t limit) {
  uint16_t cnt = 0;
  for (uint8_t i = 0; i < limit; ++i) {
    if (mask & (1u << i)) {
      cnt++;
    }
  }
  return cnt;
}

void Floppy::initChannels() {
  for (auto &ch : channels) {
    ch.buffernr = 0xff;
    ch.hasChannelName = false;
    ch.isOpen = false;
    ch.bufferidx = 0;
    ch.buffersize = 0;
  }
  for (auto &bm : bufferMeta) {
    bm.dirty = false;
    bm.track = 0;
    bm.sector = 0;
  }
  dirTrack = 0;
  dirSector = 0;
}

void Floppy::initAttach() {
  listening = false;
  talking = false;
  currentSecondary = 0;
  collectName = false;
  triggererrorchannel = false;
  triggercmdchannel = false;
  d64attached = false;
}

void Floppy::setError(uint8_t code) {
  const char *text;
  uint8_t errTrack = track;
  uint8_t errSector = sector;
  switch (code) {
  case 0:
    text = "OK";
    errTrack = 0;
    errSector = 0;
    break;
  case 1:
    text = "FILES SCRATCHED";
    break;
  case 62:
    text = "FILE NOT FOUND";
    break;
  case 65:
    text = "NO BLOCK";
    break;
  case 66:
    text = "ILLEGAL TRACK OR SECTOR";
    break;
  case 67:
    text = "ILLEGAL SYSTEM T OR S";
    break;
  case 70:
    text = "NO CHANNEL";
    break;
  case 74:
    text = "DISK FULL";
    break;
  default:
    text = "";
    break;
  }
  snprintf((char *)errmessage, sizeof(errmessage), "%02d, %s,%02d,%02d", code,
           text, errTrack, errSector);
}

bool Floppy::allocateBufferForChannel(uint8_t channel) {
  if (channels[channel].buffernr != 0xff) {
    return true;
  }
  if (channel == 15) {
    channels[15].buffernr = 4;
    return true;
  }
  for (uint8_t i = 0; i < 4; i++) {
    bool free = true;
    for (auto &ch : channels) {
      if (ch.buffernr == i) {
        free = false;
        break;
      }
    }
    if (free) {
      channels[channel].buffernr = i;
      return true;
    }
  }
  return false;
}

void Floppy::releaseBufferForChannel(uint8_t channel) {
  uint8_t bnr = channels[channel].buffernr;
  if (bnr >= 5) {
    return;
  }
  bufferMeta[bnr].dirty = false;
  bufferMeta[bnr].track = 0;
  bufferMeta[bnr].sector = 0;
  channels[channel].buffernr = 0xff;
}

bool Floppy::writeBufferToDisk(uint8_t bufferId) {
  if (!bufferMeta[bufferId].dirty) {
    return true;
  }
  int64_t off =
      calcOffset(bufferMeta[bufferId].track, bufferMeta[bufferId].sector);
  if (!d64file->seek(off, SEEK_SET)) {
    return false;
  }
  if (d64file->write(buffer[bufferId], 256) != 256) {
    return false;
  }
  bufferMeta[bufferId].dirty = false;
  return true;
}

bool Floppy::fsinitialized = false;
std::unique_ptr<FileDriver> Floppy::sysfile;

void Floppy::init(uint8_t device) {
  this->device = device;
  buffer[0] = &ram[0x300];
  buffer[1] = &ram[0x400];
  buffer[2] = &ram[0x500];
  buffer[3] = &ram[0x600];
  buffer[4] = &ram[0x700];
  if (!fsinitialized) {
    sysfile = FileSys::create();
    fsinitialized = sysfile->init();
    d64file = FileSys::create();
    for (auto &ch : channels) {
      ch.file = FileSys::create();
    }
  }
  initChannels();
  initAttach();
}

bool Floppy::attach(const std::string &filename) {
  initChannels();
  initAttach();
  std::string d64filename = Config::PATH + filename;
  d64file->close();
  if (!d64file->open(d64filename, "rb")) {
    PlatformManager::getInstance().log(
        LOG_ERROR, "Floppy", "cannot open file %s", d64filename.c_str());
    return false;
  }
  // try to detect tracks from file size
  int64_t filesize = d64file->size();
  if (filesize == -1) {
    return false;
  }
  uint8_t detectedTracks;
  if (filesize == 196608) {
    detectedTracks = 40;
  } else {
    detectedTracks = 35;
  }
  // read BAM (track 18 sector 0)
  if (!d64file->seek(calcOffset(18, 0), SEEK_SET)) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                       "unsuccessful seek operation");
    return false;
  }
  if (d64file->read(buffer[4], 256) == 0) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                       "unsuccessful read operation");
    return false;
  }
  // count free blocks (skip track 18)
  freeBlocks = 0;
  for (uint8_t t = 1; t <= detectedTracks; t++) {
    if (t == 18) {
      continue;
    }
    uint8_t base = (t - 1) * 4;
    uint32_t mask = static_cast<uint32_t>(buffer[4][base + 5]) |
                    (static_cast<uint32_t>(buffer[4][base + 6]) << 8) |
                    (static_cast<uint32_t>(buffer[4][base + 7]) << 16);
    uint16_t bits = countBitsLimited(mask, sectorsPerTrack[t]);
    freeBlocks += bits;
  }
  if (buffer[4][0] == 0) {
    return false;
  }
  dirTrack = buffer[4][0];
  dirSector = buffer[4][1];
  d64attached = true;
  return true;
}

void Floppy::detach() {
  initChannels();
  initAttach();
  d64attached = false;
  d64file->close();
}

void addDirectoryLine(uint8_t *buf, uint16_t &idx, uint16_t &addr,
                      uint16_t blocks, const std::string &name,
                      const std::string &type) {
  addr += 32;
  uint16_t endidx = idx + 32;
  // nextline pointer
  buf[idx++] = addr & 0xff;
  buf[idx++] = (addr >> 8) & 0xff;
  // line number = number of blocks
  buf[idx++] = blocks & 0xff;
  buf[idx++] = (blocks >> 8) & 0xff;
  int padding = 4 - std::to_string((int)blocks).length();
  for (uint8_t i = 0; i < padding; i++) {
    buf[idx++] = ' ';
  }
  // name + file type
  buf[idx++] = '"';
  for (char c : name) {
    buf[idx++] = static_cast<uint8_t>(c);
  }
  buf[idx++] = '"';
  padding = 17 - name.length();
  for (uint8_t i = 0; i < padding; i++) {
    buf[idx++] = ' ';
  }
  for (char c : type) {
    buf[idx++] = static_cast<uint8_t>(c);
  }
  while (idx < endidx - 1) {
    buf[idx++] = ' ';
  }
  // line terminator
  buf[idx++] = 0;
}

bool Floppy::readNextDirBlk() {
  uint8_t *buf = buffer[channels[0].buffernr];
  uint16_t idx = 0;
  switch (diriterstate) {
  case 1: {
    diriteraddr = 0x0801; // basic start
    buf[idx++] = diriteraddr & 0xff;
    buf[idx++] = (diriteraddr >> 8) & 0xff;
    diriteraddr += 30;
    // nextline pointer
    buf[idx++] = diriteraddr & 0xff;
    buf[idx++] = (diriteraddr >> 8) & 0xff;
    // disk name
    std::string diskName;
    for (uint8_t i = 0; i < 16; i++) {
      diskName.push_back(buffer[4][0x90 + i]);
    }
    // line number = number of blocks
    buf[idx++] = 0;
    buf[idx++] = 0;
    // name
    buf[idx++] = 18; // reverse on
    std::string line = "\"" + diskName + "\" ";
    for (char c : line) {
      buf[idx++] = static_cast<uint8_t>(c);
    }
    for (uint8_t i = 0; i < 5; i++) {
      uint8_t ch = buffer[4][0xa2 + i];
      buf[idx++] = ch >= 128 ? ch - 128 : ch;
    }
    buf[idx++] = 0;
    channels[0].buffersize = 32;
    channels[0].bufferidx = 0;
    diriterstate++;
    initIterateDirectoryBlk();
    break;
  }
  case 2: {
    iterateDirectoryBlk(name, buf,
                        [&](const std::string &name, const std::string &search,
                            const uint16_t blocks, const uint8_t fileType) {
                          addDirectoryLine(buf, idx, diriteraddr, blocks, name,
                                           decodeFileType(fileType));
                          return false;
                        });
    channels[0].buffersize = idx;
    channels[0].bufferidx = 0;
    if (track == 0) {
      diriterstate++;
    }
    break;
  }
  case 3: {
    diriteraddr += 30;
    buf[idx++] = diriteraddr & 0xff;
    buf[idx++] = (diriteraddr >> 8) & 0xff;
    buf[idx++] = freeBlocks & 0xff;
    buf[idx++] = (freeBlocks >> 8) & 0xff;
    std::string line = "BLOCKS FREE.";
    for (char c : line) {
      buf[idx++] = static_cast<uint8_t>(c);
    }
    for (uint8_t i = 0; i < 13; i++) {
      buf[idx++] = 32;
    }
    buf[idx++] = 0;
    buf[idx++] = 0;
    channels[0].buffersize = idx;
    channels[0].bufferidx = 0;
    diriterstate++;
    break;
  }
  case 4: {
    lastStatus = 0x40; // EOI
    channels[0].buffersize = 0;
    channels[0].bufferidx = 0;
    return true;
  }
  }
  return false;
}

bool Floppy::readNextFileBlk() {
  if (track != 0) {
    uint8_t bnr = channels[currentSecondary].buffernr;
    if (bnr >= 5) {
      lastStatus = 0x02;
      setError(70);
      return true;
    }
    if (!d64file->seek(calcOffset(track, sector), SEEK_SET)) {
      lastStatus = 0x42;
      setError(62);
      return true;
    }
    // PlatformManager::getInstance().log(LOG_INFO, TAG,
    //                                   "readNextFileBlk, currentSecondary=%d",
    //                                    currentSecondary);
    uint8_t *buf = buffer[bnr];
    uint16_t s = d64file->read(buf, 256);
    if (s == 0) {
      track = 0;
      lastStatus = 0x40; // EOI
      return true;
    }
    uint8_t oldtrack = track;
    uint8_t oldsector = sector;
    track = buf[0];
    sector = buf[1];
    bnr = channels[currentSecondary].buffernr;
    bufferMeta[bnr].track = oldtrack;
    bufferMeta[bnr].sector = oldsector;
    if ((oldtrack == track) && (oldsector == sector)) {
      PlatformManager::getInstance().log(
          LOG_ERROR, TAG, "IECIN error: track/sector repeating!");
      track = 0;
      lastStatus = 0x02;
      setError(67);
      return true;
    }
    if (track == 0) {
      channels[currentSecondary].buffersize = sector + 1;
    } else {
      channels[currentSecondary].buffersize = 256;
    }
    channels[currentSecondary].bufferidx = 2;
  } else {
    lastStatus = 0x40; // EOI
    return true;
  }
  return false;
}

bool Floppy::directLoad() {
  if (!channels[0].isOpen) {
    PlatformManager::getInstance().log(LOG_INFO, TAG, "file not found");
    lastStatus = 0x42;
    setError(62);
    return true;
  }
  uint8_t *buf = buffer[channels[0].buffernr];
  channels[0].bufferidx = 0;
  channels[0].buffersize = channels[0].file->read(buf, 256);
  if (channels[0].buffersize == 0) {
    lastStatus = 0x40; // EOI
    return true;
  }
  return false;
}

enum class CommandType { UNKNOWN, MEMORY, TEXT };

struct GeneralCommand {
  CommandType type = CommandType::UNKNOWN;
  std::string command;
  std::vector<uint8_t> rawArgs;
  std::vector<std::string> textArgs;
};

bool isMemoryCmd(const std::string &data) {
  return data.size() >= 3 && data[0] == 'M' && data[1] == '-' &&
         std::isalpha(static_cast<unsigned char>(data[2]));
}

bool isTextCmd(const std::string &data) {
  return !data.empty() && std::isalpha(static_cast<unsigned char>(data[0]));
}

std::string formatUInt8ArgsToHex(const std::vector<uint8_t> &args) {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (uint8_t i = 0; i < args.size(); i++) {
    ss << std::setw(2) << static_cast<unsigned int>(args[i]);
    if (i < args.size() - 1) {
      ss << ",";
    }
  }
  return ss.str();
}

std::string formatStringArgsToHex(const std::vector<std::string> &args) {
  std::stringstream ss;
  for (uint8_t i = 0; i < args.size(); ++i) {
    ss << args[i];
    if (i < args.size() - 1) {
      ss << ",";
    }
  }
  return ss.str();
}

std::optional<GeneralCommand> handleCommand(const std::string &data) {
  GeneralCommand cmd;
  if (isMemoryCmd(data)) {
    cmd.type = CommandType::MEMORY;
    cmd.command = data.substr(0, 3);
    cmd.rawArgs.assign(data.begin() + 3, data.end());
    return cmd;
  } else if (isTextCmd(data)) {
    cmd.type = CommandType::TEXT;
    auto pos = data.find_first_of(": ");
    if (pos == std::string::npos) {
      cmd.command = data;
      return cmd;
    }
    cmd.command = data.substr(0, pos);
    std::string params = data.substr(pos + 1);
    params.erase(0, params.find_first_not_of(" "));
    std::replace(params.begin(), params.end(), ',', ' ');
    std::stringstream ss(params);
    std::string token;
    while (ss >> token) {
      cmd.textArgs.push_back(token);
    }
    return cmd;
  }
  return std::nullopt;
}

bool Floppy::handleCmdChannel() {
  auto cmd = handleCommand(name);
  if (cmd) {
    if (cmd->type == CommandType::MEMORY) {
      uint16_t addr = cmd->rawArgs[0] + (cmd->rawArgs[1] << 8);
      PlatformManager::getInstance().log(
          LOG_INFO, TAG, "command: %s, args: %s, addr: %d",
          cmd->command.c_str(), formatUInt8ArgsToHex(cmd->rawArgs).c_str(),
          addr);
      if (cmd->command == "M-R") {
        if (channels[15].buffernr >= 5) {
          channels[15].buffernr = 4;
        }
        uint8_t *dst = buffer[channels[15].buffernr];
        uint16_t bufsize = cmd->rawArgs[2];
        if (bufsize == 0) {
          bufsize = 256;
        }
        for (uint16_t i = 0; i < bufsize && (addr + i) <= 0xffff; i++) {
          dst[i] = getMem(addr + i);
        }
        channels[15].bufferidx = 0;
        channels[15].buffersize = bufsize;
      } else if (cmd->command == "M-E") {
        exeSubroutine(addr);
      }
    } else if (cmd->type == CommandType::TEXT) {
      PlatformManager::getInstance().log(
          LOG_INFO, TAG, "command: %s, args: %s", cmd->command.c_str(),
          formatStringArgsToHex(cmd->textArgs).c_str());
      if ((cmd->command == "U1") || (cmd->command == "B-R") ||
          (cmd->command == "B-E")) {
        uint8_t track1 = static_cast<uint8_t>(std::stoul(cmd->textArgs[2]));
        uint8_t sector1 = static_cast<uint8_t>(std::stoul(cmd->textArgs[3]));
        uint8_t secch1 = static_cast<uint8_t>(std::stoul(cmd->textArgs[0]));
        if (secch1 >= 16) {
          PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                             "invalid channel: %d", secch1);
          lastStatus = 0x02;
          setError(70);
          return true;
        }
        if (channels[secch1].buffernr >= 5) {
          if (!allocateBufferForChannel(secch1)) {
            PlatformManager::getInstance().log(
                LOG_ERROR, TAG, "no free buffer for channel %d", secch1);
            lastStatus = 0x02;
            setError(70);
            return true;
          }
        }
        PlatformManager::getInstance().log(
            LOG_INFO, TAG, "track1 = %d, sector1 = %d, secch1=%d, buffernr=%d",
            (int)track1, (int)sector1, (int)secch1,
            (int)channels[secch1].buffernr);
        uint8_t addOffset = 0;
        uint16_t numOfBytes = 256;
        if ((cmd->command == "B-R") || (cmd->command == "B-E")) {
          addOffset = 2;
          numOfBytes = 254;
        }
        uint8_t bnr = channels[secch1].buffernr;
        if (bufferMeta[bnr].dirty) {
          lastStatus = 0x02;
          setError(65);
          return true;
        }
        if (!d64file->seek(calcOffset(track1, sector1) + addOffset, SEEK_SET)) {
          lastStatus = 0x02;
          setError(66);
          return true;
        }
        uint8_t *buf = buffer[bnr];
        bufferMeta[bnr].track = track1;
        bufferMeta[bnr].sector = sector1;
        if (cmd->command == "B-E") {
          d64file->read(buf, numOfBytes);
          PlatformManager::getInstance().log(LOG_INFO, TAG, "exeSubroutine %d",
                                             buf - ram);
          exeSubroutine(buf - ram);
          bufferMeta[bnr].dirty = true;
        } else {
          channels[secch1].buffersize = d64file->read(buf, numOfBytes);
          channels[secch1].bufferidx = 0;
          bufferMeta[bnr].dirty = false;
        }
      } else if (cmd->command == "B-P") {
        uint8_t secch1 = static_cast<uint8_t>(std::stoul(cmd->textArgs[0]));
        if (secch1 >= 16) {
          PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                             "invalid channel: %d", secch1);
          lastStatus = 0x02;
          setError(70);
          return true;
        }
        if (channels[secch1].buffernr >= 5) {
          if (!allocateBufferForChannel(secch1)) {
            PlatformManager::getInstance().log(
                LOG_ERROR, TAG, "no free buffer for channel %d", secch1);
            lastStatus = 0x02;
            setError(70);
            return true;
          }
        }
        uint16_t offset = static_cast<uint16_t>(std::stoul(cmd->textArgs[1]));
        if (cmd->textArgs.size() > 2) {
          offset += static_cast<uint16_t>(std::stoul(cmd->textArgs[2])) << 8;
        }
        channels[secch1].bufferidx = offset;
        PlatformManager::getInstance().log(
            LOG_INFO, TAG, "B-P channel %d, offset %d", secch1, offset);
      } else {
        PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                           "unknown command: %s", name.c_str());
      }
    } else {
      PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                         "unknown command type");
    }
  } else {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "unknown command type");
  }
  return false;
}

uint8_t Floppy::iecin() {
  if (!talking) {
    return 0;
  }
  if (triggererrorchannel) {
    if (errmessageidx < (int)sizeof(errmessage)) {
      return errmessage[errmessageidx++];
    }
    lastStatus = 0x40; // EOI
    triggererrorchannel = false;
    PlatformManager::getInstance().log(LOG_INFO, TAG, "errmessage sent!");
    return 0;
  }
  uint8_t cursec = currentSecondary;
  // PlatformManager::getInstance().log(
  //     LOG_INFO, TAG, "iecin, currentSecondary=%d", currentSecondary);
  if (name == "$") {
    cursec = 0;
  }
  lastStatus = 0;
  setError(0);
  if (!channels[cursec].isOpen) {
    lastStatus = 0x42; // file not found
    setError(62);
    return 0;
  }
  // send bytes to the c64
  if (channels[cursec].bufferidx >= channels[cursec].buffersize) {
    if (d64attached) {
      if (triggercmdchannel) {
        triggercmdchannel = false;
      } else if (name == "$") {
        if (readNextDirBlk()) {
          return 0;
        }
      } else {
        if (readNextFileBlk()) {
          return 0;
        }
      }
    } else {
      if (directLoad()) {
        return 0;
      }
    }
  }
  if (channels[cursec].buffernr >= 5) {
    if (cursec == 15) {
      channels[15].buffernr = 4;
    } else {
      if (!allocateBufferForChannel(cursec)) {
        PlatformManager::getInstance().log(
            LOG_ERROR, TAG, "iecin: cannot allocate buffer for channel %d",
            cursec);
        lastStatus = 0x02;
        setError(70);
        return 0;
      }
    }
  }
  uint8_t *buf = buffer[channels[cursec].buffernr];
  uint8_t byte = buf[channels[cursec].bufferidx];
  channels[cursec].bufferidx++;
  if (iecindebug) {
    PlatformManager::getInstance().log(LOG_INFO, TAG, "iecin byte: %x", byte);
  }
  return byte;
}

bool wildcard_match(const char *text, const char *pattern) {
  const char *retry_text = nullptr;
  const char *retry_pattern = nullptr;
  while (*text) {
    if (*pattern == '*') {
      retry_pattern = ++pattern;
      retry_text = text;
    } else if (*pattern == '?' || *text == *pattern) {
      pattern++;
      text++;
    } else if (retry_text) {
      pattern = retry_pattern;
      text = ++retry_text;
    } else {
      return false;
    }
  }
  while (*pattern == '*') {
    pattern++;
  }
  return !*pattern;
}

void Floppy::iecout(uint8_t value) {
  if (collectName) {
    if (value == 0x3f) {
      // end of filename
      collectName = false;
      PlatformManager::getInstance().log(LOG_INFO, TAG, "name = %s!",
                                         name.c_str());
      if (d64attached) {
        if (name == "$") {
          allocateBufferForChannel(0);
          channels[0].isOpen = true;
          diriterstate = 1;
        } else if (currentSecondary == 15) {
          if (listening) {
            PlatformManager::getInstance().log(LOG_INFO, TAG,
                                               "triggercmdchannel");
            handleCmdChannel();
            triggercmdchannel = true;
          }
        } else {
          // PlatformManager::getInstance().log(
          //     LOG_INFO, TAG, "iecout, currentSecondary=%d",
          //     currentSecondary);
          lastStatus = 0;
          setError(0);
          initIterateDirectoryBlk();
          bool found = false;
          while ((track != 0) && (!found)) {
            uint8_t *buf = buffer[channels[currentSecondary].buffernr];
            found = iterateDirectoryBlk(
                name, buf,
                [&](const std::string &name, const std::string &search,
                    const uint16_t blocks, const uint8_t fileType) {
                  return wildcard_match(name.c_str(), search.c_str());
                });
          }
          if (!found) {
            lastStatus = 0x42; // file not found
            setError(62);
            channels[currentSecondary].isOpen = false;
          } else {
            track = startTrack;
            sector = startSector;
            channels[currentSecondary].isOpen = true;
            channels[currentSecondary].bufferidx = 0;
            channels[currentSecondary].buffersize = 0;
          }
        }
      } else { // "direct" load
        for (auto &c : name) {
          c = tolower(c);
        }
        lastStatus = 0;
        setError(0);
        std::string filename = Config::PATH + name + ".prg";
        if (!channels[0].file->open(filename, "rb")) {
          PlatformManager::getInstance().log(LOG_INFO, TAG, "cannot open file");
          channels[0].isOpen = false;
          lastStatus = 0x42; // file not found
          setError(62);
        } else {
          channels[0].isOpen = true;
          channels[0].bufferidx = 0;
          channels[0].buffersize = 0;
          allocateBufferForChannel(0);
        }
      }
    } else {
      // get next char for filename
      name += static_cast<char>(value);
    }
  }
  if (!collectName) {
    uint8_t cmd = value & 0xf0;
    if (value == 0x20 + device) {
      listening = true;
      talking = false;
    } else if (value == 0x3f) {
      lastStatus &= 0x40;
      setError(0);
      listening = false;
    } else if (value == 0x40 + device) {
      talking = true;
      listening = false;
    } else if (value == 0x5f) {
      triggererrorchannel = false;
      talking = false;
    } else if (cmd == 0x60) {
      currentSecondary = value & 0x0f;
      if ((currentSecondary == 15) &&
          (channels[15].bufferidx >= channels[15].buffersize)) {
        if (talking) {
          errmessageidx = 0;
          channels[15].hasChannelName = false;
          channels[15].isOpen = true;
          triggererrorchannel = true;
        } else if (listening) {
          allocateBufferForChannel(15);
          channels[15].hasChannelName = true;
          channels[15].isOpen = true;
          name = "";
          collectName = true;
        }
      }
    } else if (cmd == 0xe0) {
      currentSecondary = value & 0x0f;
      releaseBufferForChannel(currentSecondary);
      channels[currentSecondary].hasChannelName = false;
      channels[currentSecondary].isOpen = false;
      name = "";
    } else if (cmd == 0xf0) {
      currentSecondary = value & 0x0f;
      if (!allocateBufferForChannel(currentSecondary)) {
        channels[currentSecondary].isOpen = false;
        PlatformManager::getInstance().log(
            LOG_ERROR, TAG, "no free buffer for channel %d", currentSecondary);
        return;
      }
      channels[currentSecondary].hasChannelName = true;
      channels[currentSecondary].isOpen = true;
      name = "";
      collectName = true;
    }
  }
}

uint16_t Floppy::load(const std::string &filename, uint8_t *ram) {
  std::string path = Config::PATH + filename;
  if (!sysfile->open(path, "rb")) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "cannot open file %s",
                                       path.c_str());
    return 0;
  }
#if defined(BOARD_CYD)
  vTaskDelay(0);
#endif
  uint8_t buf[256];
  if (sysfile->read(buf, 2) != 2) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                       "not a prg file (header too short)");
    return 0;
  }
  uint8_t addrLow = buf[0], addrHigh = buf[1];
  uint16_t addr = addrLow | (addrHigh << 8);
  uint16_t bytesRead;
#if defined(BOARD_CYD)
  vTaskDelay(0);
#endif
  while ((bytesRead = sysfile->read(buf, sizeof(buf))) > 0) {
#if defined(BOARD_CYD)
    vTaskDelay(0);
#endif
    for (uint16_t i = 0; i < bytesRead; i++) {
      ram[addr++] = buf[i];
    }
  }
  sysfile->close();
#if defined(BOARD_CYD)
  vTaskDelay(100);
#endif
  return addr;
}

bool Floppy::save(const std::string &filename, uint8_t *ram, uint16_t startaddr,
                  uint16_t endaddr) {
  std::string path = Config::PATH + filename;
  if (!sysfile->open(path, "wb")) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "cannot open file %s",
                                       path.c_str());
    return false;
  }
  uint8_t buf[2];
  buf[0] = startaddr & 0xff;
  buf[1] = (startaddr >> 8) & 0xff;
  if (sysfile->write(buf, 2) != 2) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "could not write file");
    return false;
  }
  uint16_t length = endaddr - startaddr;
  if (sysfile->write(ram + startaddr, length) != length) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "could not write file");
    return false;
  }
  sysfile->close();
  return true;
}

void Floppy::rmPrgFromFilename(std::string &filename) {
  if (filename.size() > 4 &&
      filename.compare(filename.size() - 4, 4, ".prg") == 0) {
    filename.erase(filename.size() - 4);
  }
}

bool Floppy::listnextentry(std::string &name, bool start) {
  return sysfile->listnextentry(name, start);
}

uint8_t Floppy::getMem(uint16_t addr) {
  if (addr < 0x0800) {
    return ram[addr];
  } else if (addr >= 0xc000) {
    return rom_1541[addr - 0xc000];
  }
  return 0;
}

void Floppy::setMem(uint16_t addr, uint8_t val) {
  if (addr < 0x0800) {
    ram[addr] = val;
  }
}

void Floppy::logDebugInfo() {
  PlatformManager::getInstance().log(LOG_INFO, TAG,
                                     "pc: %2x, cmd: %s, a: %x, x: %x, y: %x, "
                                     "sp: %x, sr: %x, arg1 = %x, arg2 = %x",
                                     pc, cmdName[getMem(pc)], a, x, y, sp, sr,
                                     getMem(pc + 1), getMem(pc + 2));
}

void Floppy::exeSubroutine(uint16_t regpc) {
  sp = 0xff;
  uint16_t i = 20000;
  uint8_t tsp = sp;
  pc = regpc;
  while (i != 0) {
    i--;
    logDebugInfo();
    uint8_t nextopc = getMem(pc++);
    if ((nextopc == 0x00) || ((sp == tsp) && (nextopc == 0x60))) { // brk or rts
      break;
    }
    execute(nextopc);
  }
}

void Floppy::run() {}
