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
#include "fs/FileDriver.h"
#include "keyboard/C64Keycodes.h"
#include "keyboard/CodeTripleDef.h"
#include "nlohmann/json.hpp"
#include "platform/PlatformManager.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

using json = nlohmann::json;

/*
example json file:
{
  "version": 1,

  "autostart": "dkong",

  "sdlkeyboardlayout": "ch",

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

bool lookupC64Keycode(std::string_view name, CodeTripleS &out);

void from_json(const json &j, CodeTripleS &ct);
void from_json(const json &j, JoystickOnlyTextKeycode &tk);

struct JoystickOnlyConfig {
  std::vector<JoystickOnlyTextKeycode> keycodes;
};
void from_json(const json &j, JoystickOnlyConfig &cfg);

struct RootConfig {
  int version = 1;
  std::string autostart;
  std::string sdlkeyboardlayout;
  JoystickOnlyConfig joystickOnly;
};
void from_json(const json &j, RootConfig &cfg);

class FileConfig {
private:
  static bool loadConfigCalled;
  static bool configAvailable;
  static nlohmann::json configJson;
  static RootConfig cfg;

  static bool readFileToString(FileDriver &fd, const std::string &path,
                               std::string &out);

public:
  static void loadConfig(FileDriver &fd, const std::string &filename);
  static std::string getAutostartGame();
  static std::string getSdlKeyboardLayout();
  static std::vector<JoystickOnlyTextKeycode> getJoystickOnlyKeycodes();
};

#endif // FILECONFIG_H
