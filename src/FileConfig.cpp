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
#include "FileConfig.h"
#include <stdexcept>

bool FileConfig::loadConfigCalled = false;
bool FileConfig::configAvailable = false;
nlohmann::json FileConfig::configJson = nlohmann::json::object();
RootConfig FileConfig::cfg;

bool lookupC64Keycode(std::string_view name, CodeTripleS &out) {
  for (const auto &e : C64_KEYCODES) {
    if (name == e.name) {
      out = e.value;
      return true;
    }
  }
  return false;
}

void from_json(const json &j, CodeTripleS &ct) {
  if (j.is_string()) {
    const auto &name = j.get<std::string>();
    if (!lookupC64Keycode(name, ct)) {
      PlatformManager::getInstance().log(
          LOG_INFO, "FileConfig", "Unknown C64 keycode: %s", name.c_str());
      throw std::runtime_error("Unknown C64 keycode: " + name);
    }
    return;
  }
  if (j.is_array() && j.size() == 3) {
    ct = CodeTripleS{j.at(0).get<uint8_t>(), j.at(1).get<uint8_t>(),
                     j.at(2).get<uint8_t>()};
    return;
  }
  PlatformManager::getInstance().log(LOG_INFO, "FileConfig",
                                     "Invalid C64 keycode format");
  throw std::runtime_error("Invalid C64 keycode format");
}

void from_json(const json &j, JoystickOnlyTextKeycode &tk) {
  const json &jt = j.at("text");
  if (!jt.is_array()) {
    PlatformManager::getInstance().log(
        LOG_INFO, "FileConfig", "'text' must be a numeric CodeTriple array");
    throw std::runtime_error("'text' must be a numeric CodeTriple array");
  }
  jt.get_to(tk.text);
  j.at("c64keycode").get_to(tk.keycode);
}

void from_json(const json &j, JoystickOnlyConfig &cfg) {
  if (j.contains("keycodes")) {
    cfg.keycodes = j.at("keycodes").get<std::vector<JoystickOnlyTextKeycode>>();
  }
}

void from_json(const json &j, RootConfig &cfg) {
  cfg.version = j.value("version", 1);
  cfg.autostart = j.value("autostart", std::string{});
  cfg.sdlkeyboardlayout = j.value("sdlkeyboardlayout", std::string{});
  if (j.contains("joystickOnly")) {
    cfg.joystickOnly = j.at("joystickOnly").get<JoystickOnlyConfig>();
  }
}

bool FileConfig::readFileToString(FileDriver &fd, const std::string &path,
                                  std::string &out) {
  if (!fd.init())
    return false;
  if (!fd.open(path, "r"))
    return false;

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

void FileConfig::loadConfig(FileDriver &fd, const std::string &filename) {
  if (loadConfigCalled)
    return;
  loadConfigCalled = true;

  std::string configData;
  if (!readFileToString(fd, filename, configData)) {
    configAvailable = false;
    return;
  }

  try {
    configJson = nlohmann::json::parse(configData);
    PlatformManager::getInstance().log(LOG_INFO, "FileConfig", "config loaded");
    configAvailable = true;
  } catch (const nlohmann::json::exception &) {
    PlatformManager::getInstance().log(LOG_INFO, "FileConfig",
                                       "error loading config");
    configAvailable = false;
  }
}

std::string FileConfig::getAutostartGame() {
  if (!configAvailable)
    return {};
  try {
    return configJson.get<RootConfig>().autostart;
  } catch (...) {
    return {};
  }
}

std::string FileConfig::getSdlKeyboardLayout() {
  if (!configAvailable)
    return {};
  try {
    return configJson.get<RootConfig>().sdlkeyboardlayout;
  } catch (...) {
    return {};
  }
}

std::vector<JoystickOnlyTextKeycode> FileConfig::getJoystickOnlyKeycodes() {
  if (!configAvailable)
    return {};
  try {
    return configJson.get<RootConfig>().joystickOnly.keycodes;
  } catch (...) {
    return {};
  }
}
