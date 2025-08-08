#ifdef PLATFORM_LINUX
#include "C64Emu.h"
#include "platform/PlatformManager.h"

static const char *TAG = "c64linux";

C64Emu c64Emu;

int main() {
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
