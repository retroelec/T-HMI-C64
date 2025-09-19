#if defined(PLATFORM_LINUX) || defined(_WIN32)
#include "C64Emu.h"
#include "platform/PlatformManager.h"

static const char *TAG = "c64linux";

C64Emu c64Emu;

int main(int argc, char *argv[]) {
  // parse arguments
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "-scale" && i + 1 < argc) {
      int val = std::atoi(argv[i + 1]);
      if (val >= 1 && val <= 8) {
        Config::LCDSCALE = val;
      }
      i++;
    }
  }

  // start emulator
  try {
    c64Emu.setup();
  } catch (...) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "setup() failed");
    return EXIT_FAILURE;
  }
  PlatformManager::getInstance().log(LOG_INFO, TAG, "starting emulator");
  while (true) {
    c64Emu.loop();
  }
}
#endif
