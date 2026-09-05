#if defined(PLATFORM_LINUX) || defined(_WIN32)
#include "C64Emu.h"
#include "platform/PlatformFactory.h"
#include "platform/PlatformManager.h"

static const char *TAG = "c64linux";

int savedArgc = 0;
char **savedArgv = nullptr;

C64Emu c64Emu;

int main(int argc, char *argv[]) {
  savedArgc = argc;
  savedArgv = argv;

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

  // init platform manager
  PlatformManager::initialize(PlatformNS::create());

  // start emulator
  try {
    c64Emu.setup();
  } catch (const std::exception &e) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG, "setup() failed: %s",
                                       e.what());
    return EXIT_FAILURE;
  } catch (...) {
    PlatformManager::getInstance().log(LOG_ERROR, TAG,
                                       "setup() failed (unknown exception)");
    return EXIT_FAILURE;
  }
  PlatformManager::getInstance().log(LOG_INFO, TAG, "starting emulator");
  while (true) {
    c64Emu.loop();
  }
}
#endif
