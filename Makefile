# choose board
BOARD := T_HMI
#BOARD := T_DISPLAY_S3
#BOARD := WAVESHARE
#BOARD := CYD
#BOARD := LEDMATRIX

# choose keyboard
KEYBOARD := BLE_KEYBOARD
#KEYBOARD := WEB_KEYBOARD
#KEYBOARD := NO_KEYBOARD

ifeq ($(BOARD), CYD)
  ifneq ($(KEYBOARD), NO_KEYBOARD)
    $(error FEHLER: If BOARD is set to CYD, KEYBOARD MUST have the value NO_KEYBOARD. (current: $(KEYBOARD)))
  endif
endif

UNAME_S := $(shell uname -s)

# choose port
ifeq ($(UNAME_S), Linux)
  ifeq ($(BOARD), CYD)
    PORT := /dev/ttyUSB0
  else
    PORT := /dev/ttyACM0
  endif
else ifeq ($(UNAME_S), Darwin)
  PORT := /dev/tty.usbmodem1101
endif

ALLBOARDS := T_HMI T_DISPLAY_S3 WAVESHARE
ALLKEYBOARDS := BLE_KEYBOARD WEB_KEYBOARD

BUILD_EXTRA_FLAGS := -DBOARD_$(BOARD) -DUSE_$(KEYBOARD) -DESP32

ifeq ($(BOARD), WAVESHARE)
  FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,DFUOnBoot=dfu,FlashSize=16M,JTAGAdapter=builtin,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,DebugLevel=info
  BUILD_EXTRA_FLAGS += -DBOARD_HAS_PSRAM
else ifeq ($(BOARD), LEDMATRIX)
  FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=8M,PSRAM=opi,PartitionScheme=default_8MB
  BUILD_EXTRA_FLAGS += -DBOARD_HAS_PSRAM
else
  ifeq ($(BOARD),CYD)
    FQBN := esp32:esp32:esp32:FlashMode=qio,FlashSize=4M,PartitionScheme=huge_app,LoopCore=0,DebugLevel=info
  else
    FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,DFUOnBoot=dfu,FlashSize=16M,JTAGAdapter=builtin,PartitionScheme=huge_app,PSRAM=opi,DebugLevel=info
    BUILD_EXTRA_FLAGS += -DBOARD_HAS_PSRAM
  endif
endif

.DEFAULT_GOAL := compile

SRCDIR := src
SOURCEFILES := $(shell find src -name '*.cpp')
HEADERFILES := $(shell find src -name '*.h')

CLI_COMPILE = compile \
	--library . \
	--warnings all \
	--fqbn $(FQBN) \
	--build-property "build.extra_flags=$(BUILD_EXTRA_FLAGS)" \
	--build-path $(BUILD_PATH)

compile: SKETCH=arduino/C64Emu/C64Emu.ino
compile: BUILD_PATH=build-$(BOARD)-$(KEYBOARD)
compile: $(SOURCEFILES) $(HEADERFILES)
	arduino-cli $(CLI_COMPILE) $(SKETCH)

compileBLEJoystick: SKETCH=arduino/BLEJoystick/BLEJoystick.ino
compileBLEJoystick: BUILD_PATH=build-$(BOARD)-BLEJoystick
compileBLEJoystick: $(SOURCEFILES) $(HEADERFILES)
	arduino-cli $(CLI_COMPILE) $(SKETCH)

clean:
	rm -rf build-$(BOARD)-$(KEYBOARD)

compileAll:
	@for board in $(ALLBOARDS); do \
		for kbd in $(ALLKEYBOARDS); do \
			echo "\nCompiling for Board: $$board | Keyboard: $$kbd"; \
			$(MAKE) clean compile BOARD=$$board KEYBOARD=$$kbd || exit $$?; \
		done \
	done
	echo "\nCompiling for Board: CYD"; \
	$(MAKE) clean compile BOARD=CYD KEYBOARD=NO_KEYBOARD || exit $$?; \
	echo "\nCompiling for LEDMatrix"; \
	$(MAKE) clean compile BOARD=LEDMATRIX KEYBOARD=BLE_KEYBOARD || exit $$?; \

# first you have to get the docker image:
# podman pull docker.io/retroelec42/arduino-cli-thmic64:latest
podcompile:	arduino/C64Emu/C64Emu.ino $(SOURCEFILES) $(HEADERFILES)
	podman run -it --rm -v .:/workspace/T-HMI-C64 arduino-cli-thmic64 $(CLI_COMPILE)

upload:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) -i build-$(BOARD)-$(KEYBOARD)/C64Emu.ino.bin

uploadBLEJoystick:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) -i build-$(BOARD)-BLEJoystick/BLEJoystick.ino.bin

# create file $HOME/.minirc.dfl 
# content:
# pu addcarreturn    Yes
monitor:
	minicom -D $(PORT) -b 115200

format:
	@find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i
	@find arduino -name "*.ino" | xargs clang-format -i

# see https://arduino.github.io/arduino-cli/1.0/getting-started/
install:	check_install
	arduino-cli config init --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json --overwrite
	arduino-cli config set network.connection_timeout 600s
	arduino-cli core update-index
	arduino-cli core install esp32:esp32@3.2.0
	arduino-cli lib install ArduinoJson # used to read config file
	arduino-cli config set library.enable_unsafe_install true
	arduino-cli lib install --git-url https://github.com/ESP32Async/AsyncTCP.git # used for web keyboard
	arduino-cli lib install --git-url https://github.com/ESP32Async/ESPAsyncWebServer.git # used for web keyboard
	arduino-cli lib install --git-url https://github.com/devyte/ESPAsyncDNSServer.git # used for web keyboard
	arduino-cli lib install --git-url https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA.git # used for LED matrix panel
	arduino-cli lib install --git-url https://github.com/adafruit/Adafruit-GFX-Library.git # used for LED matrix panel
	arduino-cli lib install --git-url https://github.com/adafruit/Adafruit_BusIO # used for LED matrix panel

check_install:
	@echo -n "Are you sure? [y/N] " && read ans && [ $${ans:-N} = y ]


# Linux

BUILDDIRLINUX := buildlinux
TARGETLINUX := c64linux

OBJFILESLINUX := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIRLINUX)/%.o,$(SOURCEFILES))

CXX_LINUX := g++
CXXFLAGS_LINUX := -std=c++17 -Wall -MMD -MP -DPLATFORM_LINUX -Isrc
LDFLAGS_LINUX := -lSDL2 -pthread

check_linux:
	@if [ "$(UNAME_S)" != "Linux" ]; then \
		echo "Error: $(TARGETLINUX) can only be built on linux!"; \
		exit 1; \
	fi

$(TARGETLINUX):	check_linux $(OBJFILESLINUX)
	$(CXX_LINUX) $(OBJFILESLINUX) -o $@ $(LDFLAGS_LINUX)

$(BUILDDIRLINUX)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX_LINUX) $(CXXFLAGS_LINUX) -c $< -o $@

-include $(OBJFILESLINUX:.o=.d)

cleanlinux:
	rm -rf $(BUILDDIRLINUX) $(TARGETLINUX)

.PHONY: cleanlinux


# macOS

BUILDDIRMAC := buildmac
TARGETMAC := c64mac

OBJFILESMAC := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIRMAC)/%.o,$(SOURCEFILES))

CXX_MAC := g++
CXXFLAGS_MAC := -std=c++17 -Wall -MMD -MP -DPLATFORM_LINUX -Isrc -I/opt/homebrew/include/ -D_THREAD_SAFE
LDFLAGS_MAC := -L/opt/homebrew/lib -lSDL2 -pthread

check_mac:
	@if [ "$(UNAME_S)" != "Darwin" ]; then \
		echo "Error: $(TARGETMAC) can only be built on macOS!"; \
		exit 1; \
	fi

$(TARGETMAC):	check_mac $(OBJFILESMAC)
	$(CXX_MAC) $(OBJFILESMAC) -o $@ $(LDFLAGS_MAC)

$(BUILDDIRMAC)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX_MAC) $(CXXFLAGS_MAC) -c $< -o $@

-include $(OBJFILESMAC:.o=.d)

cleanmac:
	rm -rf $(BUILDDIRMAC) $(TARGETMAC)

.PHONY: cleanlinux cleanmac


# Windows
# first you have to get the docker image:
# podman pull docker.io/retroelec42/sdl2-cross:latest
c64win.exe:
	make -f windows/Makefile clean
	podman run -it --rm -v "$(PWD)":/build sdl2-cross make -f windows/Makefile

copydlls:
	podman create --name tmp sdl2-cross:latest
	podman cp tmp:/usr/lib/gcc/x86_64-w64-mingw32/12-posix/libstdc++-6.dll windows/
	podman cp tmp:/usr/lib/gcc/x86_64-w64-mingw32/12-posix/libgcc_s_seh-1.dll windows/
	podman cp tmp:/usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll windows/
	podman cp tmp:/opt/sdl2-windows/SDL2/x86_64-w64-mingw32/bin/SDL2.dll windows/
	podman rm tmp

