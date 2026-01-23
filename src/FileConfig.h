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
#ifndef FILECONFIG_H
#define FILECONFIG_H

#include "JoystickOnlyTextKeycode.h"
#include "keyboard/C64Keycodes.h"
#include "keyboard/CodeTripleDef.h"
#include "nlohmann/json.hpp"
#include "platform/PlatformManager.h"
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

using json = nlohmann::json;

/*
example json file:
{
  "version": 1,

  "joystickOnly": {
    "keycodes": [
      {
        "text":    [32, 6, 53],
        "c64keycode": "C64_KEYCODE_F5"
      },
      {
        "text":    [32, 6, 55],
        "c64keycode": "C64_KEYCODE_F7"
      }
    ]
  }
}
*/

inline bool lookupC64Keycode(std::string_view name, CodeTripleS &out) {
  for (const auto &e : C64_KEYCODES) {
    if (name == e.name) {
      out = e.value;
      return true;
    }
  }
  return false;
}

inline void from_json(const json &j, CodeTripleS &ct) {
  // variant A: symbolic name
  if (j.is_string()) {
    const auto &name = j.get<std::string>();
    if (!lookupC64Keycode(name, ct)) {
      PlatformManager::getInstance().log(
          LOG_INFO, "FileConfig", "Unknown C64 keycode: %s", name.c_str());
      throw std::runtime_error("Unknown C64 keycode: " + name);
    }
    return;
  }
  // variant B: explicit codes
  if (j.is_array() && j.size() == 3) {
    ct = CodeTripleS{j.at(0).get<uint8_t>(), j.at(1).get<uint8_t>(),
                     j.at(2).get<uint8_t>()};
    return;
  }
  PlatformManager::getInstance().log(LOG_INFO, "FileConfig",
                                     "Invalid C64 keycode format");
  throw std::runtime_error("Invalid C64 keycode format");
}

inline void from_json(const json &j, JoystickOnlyTextKeycode &tk) {
  // array only
  const json &jt = j.at("text");
  if (!jt.is_array()) {
    PlatformManager::getInstance().log(
        LOG_INFO, "FileConfig", "text' must be a numeric CodeTriple array");
    throw std::runtime_error("text' must be a numeric CodeTriple array");
  }
  jt.get_to(tk.text);
  // keycode: array or string
  j.at("c64keycode").get_to(tk.keycode);
}

struct JoystickOnlyConfig {
  std::vector<JoystickOnlyTextKeycode> keycodes;
};

inline void from_json(const json &j, JoystickOnlyConfig &cfg) {
  if (j.contains("keycodes")) {
    cfg.keycodes = j.at("keycodes").get<std::vector<JoystickOnlyTextKeycode>>();
  }
}

struct RootConfig {
  int version = 1;
  JoystickOnlyConfig joystickOnly;
};

inline void from_json(const json &j, RootConfig &cfg) {
  cfg.version = j.value("version", 1);
  if (j.contains("joystickOnly")) {
    cfg.joystickOnly = j.at("joystickOnly").get<JoystickOnlyConfig>();
  }
}

class FileConfig {
private:
  inline static bool configAvailable = false;
  inline static nlohmann::json configJson = nlohmann::json::object();
  inline static struct RootConfig cfg;

  static bool readFileToString(FileDriver &fd, const std::string &path,
                               std::string &out) {
    if (!fd.init()) {
      return false;
    }
    if (!fd.open(path, "r")) {
      return false;
    }
    const int64_t fileSize = fd.size();
    if (fileSize <= 0) {
      fd.close();
      return false;
    }
    out.resize(static_cast<size_t>(fileSize));
    size_t bytesRead = fd.read(out.data(), out.size());
    fd.close();
    return bytesRead == out.size();
  }

public:
  static std::vector<JoystickOnlyTextKeycode> getJoystickOnlyKeycodes() {
    if (!configAvailable) {
      return {};
    }
    try {
      RootConfig cfg = configJson.get<RootConfig>();
      return cfg.joystickOnly.keycodes;
    } catch (...) {
      return {};
    }
  }

  static void loadConfig(FileDriver &fd, const std::string &filename) {
    std::string config;
    if (!readFileToString(fd, filename, config)) {
      configAvailable = false;
    }
    try {
      configJson = nlohmann::json::parse(config);
      PlatformManager::getInstance().log(LOG_INFO, "FileConfig",
                                         "config loaded");
      configAvailable = true;
    } catch (const nlohmann::json::exception &) {
      PlatformManager::getInstance().log(LOG_INFO, "FileConfig",
                                         "error loading config");
      configAvailable = false;
    }
  }
};
#endif // FILECONFIG_H
