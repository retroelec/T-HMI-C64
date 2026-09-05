# choose board
BOARD := T_HMI
#BOARD := T_DISPLAY_S3
#BOARD := WAVESHARE
#BOARD := CYD
#BOARD := LEDMATRIX1
#BOARD := LEDMATRIX2
#BOARD := ESP32_C3

# choose keyboard
#KEYBOARD := BLE_KEYBOARD
#KEYBOARD := WEB_KEYBOARD
KEYBOARD := BLEWEB_KEYBOARD
#KEYBOARD := NO_KEYBOARD

ifeq ($(BOARD), CYD)
  ifneq ($(KEYBOARD), NO_KEYBOARD)
    $(error ERROR: If BOARD is set to CYD, KEYBOARD MUST have the value NO_KEYBOARD. (current: $(KEYBOARD)))
  endif
endif

ifeq ($(BOARD), ESP32_C3)
  ifneq ($(KEYBOARD), BLE_KEYBOARD)
    $(error ERROR: If BOARD is set to ESP32_C3, KEYBOARD MUST have the value BLE_KEYBOARD. (current: $(KEYBOARD)))
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
else ifeq ($(BOARD), LEDMATRIX1)
  # ESP32-S3-WROOM1 N8R2 -> (2 MB Quad-SPI PSRAM) -> PSRAM=enabled
  FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=8M,PSRAM=enabled,PartitionScheme=default_8MB
  BUILD_EXTRA_FLAGS += -DBOARD_HAS_PSRAM
else ifeq ($(BOARD), LEDMATRIX2)
  # ESP32-S3-WROOM1 N16R8 -> (8 MB Octal-SPI PSRAM) -> PSRAM=opi
  FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=8M,PSRAM=opi,PartitionScheme=default_8MB
  BUILD_EXTRA_FLAGS += -DBOARD_HAS_PSRAM
else ifeq ($(BOARD), CYD)
  FQBN := esp32:esp32:esp32:FlashMode=qio,FlashSize=4M,PartitionScheme=huge_app,LoopCore=0,DebugLevel=info
  BUILD_EXTRA_FLAGS += -O2
else ifeq ($(BOARD), ESP32_C3)
  FQBN := esp32:esp32:lolin_c3_pico:DebugLevel=info
else
  FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,DFUOnBoot=dfu,FlashSize=16M,JTAGAdapter=builtin,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,DebugLevel=info
  BUILD_EXTRA_FLAGS += -DBOARD_HAS_PSRAM
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

ifeq ($(BOARD), ESP32_C3)
compile:
	@echo "ERROR: For ESP32_C3 board, only compileBLEJoystick and uploadBLEJoystick are supported"
	@echo "       Please use: make compileBLEJoystick"
	@exit 1
else
compile: SKETCH=arduino/C64Emu/C64Emu.ino
compile: BUILD_PATH=build-$(BOARD)-$(KEYBOARD)
compile: $(SOURCEFILES) $(HEADERFILES)
	arduino-cli $(CLI_COMPILE) $(SKETCH)
endif

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
	$(MAKE) clean compile BOARD=LEDMATRIX1 KEYBOARD=BLE_KEYBOARD || exit $$?; \
	$(MAKE) clean compile BOARD=LEDMATRIX2 KEYBOARD=BLE_KEYBOARD || exit $$?; \

# first you have to get the docker image:
# podman pull docker.io/retroelec42/arduinocli-thmic64:3.3.10 (resp. 3.2.0 for the CYD board)
POD_IMAGE := docker.io/retroelec42/arduinocli-thmic64:3.3.10
ifeq ($(BOARD), CYD)
POD_IMAGE := docker.io/retroelec42/arduinocli-thmic64:3.2.0
endif
podcompile: SKETCH=arduino/C64Emu/C64Emu.ino
podcompile: BUILD_PATH=build-$(BOARD)-$(KEYBOARD)
podcompile: $(SOURCEFILES) $(HEADERFILES)
	podman run -it --rm -v .:/workspace/T-HMI-C64 $(POD_IMAGE) $(CLI_COMPILE) $(SKETCH)

podcompileBLEJoystick: SKETCH=arduino/BLEJoystick/BLEJoystick.ino
podcompileBLEJoystick: BUILD_PATH=build-$(BOARD)-BLEJoystick
podcompileBLEJoystick: $(SOURCEFILES) $(HEADERFILES)
	podman run -it --rm -v .:/workspace/T-HMI-C64 docker.io/retroelec42/arduinocli-thmic64:3.3.10 $(CLI_COMPILE) $(SKETCH)

ifeq ($(BOARD), ESP32_C3)
upload:
	@echo "ERROR: For ESP32_C3 board, only uploadBLEJoystick is supported"
	@echo "       Please use: make uploadBLEJoystick"
	@exit 1

uploadOTA:
	@echo "ERROR: For ESP32_C3 board, OTA upload is not supported"
	@echo "       Please use: make uploadBLEJoystick"
	@exit 1
else
upload:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) -i build-$(BOARD)-$(KEYBOARD)/C64Emu.ino.bin

uploadOTA:
	arduino-cli upload -p $$(arduino-cli board list | grep network | awk '{ print $$1 }') --fqbn $(FQBN) --discovery-timeout 30s --upload-field password= -i build-$(BOARD)-$(KEYBOARD)/C64Emu.ino.bin
endif

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
	arduino-cli core install esp32:esp32@3.3.10
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
BUILDDIRNCTERM := buildncterm
TARGETNCTERM := c64ncterm
BUILDDIRTEXTTERM := buildtextterm
TARGETTEXTTERM := c64textterm

OBJFILESLINUX := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIRLINUX)/%.o,$(SOURCEFILES))
OBJFILESNCTERM := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIRNCTERM)/%.o,$(SOURCEFILES))
OBJFILESTEXTTERM := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIRTEXTTERM)/%.o,$(SOURCEFILES))

CXX_LINUX := g++
CXXFLAGS_LINUX := -g -O0 -std=c++17 -Wall -MMD -MP -DPLATFORM_LINUX -Isrc
LDFLAGS_LINUX := -lSDL2 -pthread

CXXFLAGS_NCTERM := -g -O0 -std=c++17 -Wall -MMD -MP -DPLATFORM_LINUX -DLINUX_NCTERM -Isrc
LDFLAGS_NCTERM := -lnotcurses-core -lnotcurses -lSDL2 -pthread

CXXFLAGS_TEXTTERM := -g -O0 -std=c++17 -Wall -MMD -MP -DPLATFORM_LINUX -DLINUX_TEXTTERM -Isrc
LDFLAGS_TEXTTERM := -pthread

check_linux:
	@if [ "$(UNAME_S)" != "Linux" ]; then \
		echo "Error: Linux targets can only be built on linux!"; \
		exit 1; \
	fi

$(TARGETLINUX):	check_linux $(OBJFILESLINUX)
	$(CXX_LINUX) $(OBJFILESLINUX) -o $@ $(LDFLAGS_LINUX)

$(TARGETNCTERM):	check_linux $(OBJFILESNCTERM)
	$(CXX_LINUX) $(OBJFILESNCTERM) -o $@ $(LDFLAGS_NCTERM)

$(TARGETTEXTTERM):	check_linux $(OBJFILESTEXTTERM)
	$(CXX_LINUX) $(OBJFILESTEXTTERM) -o $@ $(LDFLAGS_TEXTTERM)

$(BUILDDIRLINUX)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX_LINUX) $(CXXFLAGS_LINUX) -c $< -o $@

$(BUILDDIRNCTERM)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX_LINUX) $(CXXFLAGS_NCTERM) -c $< -o $@

$(BUILDDIRTEXTTERM)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX_LINUX) $(CXXFLAGS_TEXTTERM) -c $< -o $@

-include $(OBJFILESLINUX:.o=.d)
-include $(OBJFILESNCTERM:.o=.d)
-include $(OBJFILESTEXTTERM:.o=.d)

cleanlinux:
	rm -rf $(BUILDDIRLINUX) $(BUILDDIRNCTERM) $(BUILDDIRTEXTTERM) $(TARGETLINUX) $(TARGETNCTERM) $(TARGETTEXTTERM)

.PHONY: cleanlinux


# RPi LED Matrix (needs hzeller's rpi-rgb-led-matrix library)
#
# Build:
#   git clone https://github.com/hzeller/rpi-rgb-led-matrix.git
#   cd rpi-rgb-led-matrix && make -j4
#
#   cd T-HMI-C64 && make c64rpiled
#
# Override library paths if needed:
#   make c64rpiled CXXFLAGS_RPILED_EXTRA="-I/opt/matrix/include" \
#                 LDFLAGS_RPILED_EXTRA="-L/opt/matrix/lib"

BUILDDIRRPILED := buildrpiled
TARGETRPILED := c64rpiled

OBJFILESRPILED := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIRRPILED)/%.o,$(SOURCEFILES))

CXXFLAGS_RPILED_EXTRA ?= -I../rpi-rgb-led-matrix/include
LDFLAGS_RPILED_EXTRA ?= -L../rpi-rgb-led-matrix/lib

CXXFLAGS_RPILED := -g -O2 -std=c++17 -Wall -MMD -MP -DPLATFORM_LINUX -DLINUX_RPILED -Isrc $(CXXFLAGS_RPILED_EXTRA)
LDFLAGS_RPILED := -lrgbmatrix -lSDL2 -pthread $(LDFLAGS_RPILED_EXTRA)

$(TARGETRPILED):	check_linux $(OBJFILESRPILED)
	$(CXX) $(OBJFILESRPILED) -o $@ $(LDFLAGS_RPILED)

$(BUILDDIRRPILED)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS_RPILED) -c $< -o $@

-include $(OBJFILESRPILED:.o=.d)

cleanrpiled:
	rm -rf $(BUILDDIRRPILED) $(TARGETRPILED)

.PHONY: cleanrpiled


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

