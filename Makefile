# choose board
BOARD := T_HMI
#BOARD := T_DISPLAY_S3
#BOARD := WAVESHARE

# choose keyboard
KEYBOARD := BLE_KEYBOARD
#KEYBOARD := WEB_KEYBOARD

UNAME_S := $(shell uname -s)

# choose port
ifeq ($(UNAME_S),Linux)
  PORT := /dev/ttyACM0
else ifeq ($(UNAME_S),Darwin)
  PORT := /dev/tty.usbmodem1101
endif

ALLBOARDS := T_HMI T_DISPLAY_S3 WAVESHARE
ALLKEYBOARDS := BLE_KEYBOARD WEB_KEYBOARD

ifeq ($(BOARD), WAVESHARE)
  FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,DFUOnBoot=dfu,FlashSize=16M,JTAGAdapter=builtin,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,DebugLevel=info
else
  FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,DFUOnBoot=dfu,FlashSize=16M,JTAGAdapter=builtin,PartitionScheme=huge_app,PSRAM=opi,DebugLevel=info
endif

.DEFAULT_GOAL := compile

SRCDIR := src
SOURCEFILES := $(shell find src -name '*.cpp')
HEADERFILES := $(shell find src -name '*.h')

TARGET := build$(BOARD)/T-HMI-C64.ino.elf

CLI_COMPILE := compile --warnings all --fqbn $(FQBN) --build-property "build.extra_flags=-DBOARD_$(BOARD) -DUSE_$(KEYBOARD) -DESP32" --build-path build-$(BOARD)-$(KEYBOARD) T-HMI-C64.ino

# -DESP32 is needed for ESP_Async_WebServer, Async_TCP
$(TARGET):	T-HMI-C64.ino $(SOURCEFILES) $(HEADERFILES) Makefile
	arduino-cli $(CLI_COMPILE)

compile:	$(TARGET)

clean:
	rm -rf build-$(BOARD)-$(KEYBOARD)

compileAll:
	@for board in $(ALLBOARDS); do \
		for kbd in $(ALLKEYBOARDS); do \
			echo "\nCompiling for Board: $$board | Keyboard: $$kbd"; \
			$(MAKE) clean compile BOARD=$$board KEYBOARD=$$kbd || exit $$?; \
		done \
	done

# first you have to get the docker image:
# podman pull docker.io/retroelec42/arduino-cli-thmic64:latest
podcompile:	T-HMI-C64.ino $(SOURCEFILES) $(HEADERFILES)
	podman run -it --rm -v .:/workspace/T-HMI-C64 arduino-cli-thmic64 $(CLI_COMPILE)

upload:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) -i build-$(BOARD)-$(KEYBOARD)/T-HMI-C64.ino.bin

# create file $HOME/.minirc.dfl 
# content:
# pu addcarreturn    Yes
monitor:
	minicom -D $(PORT) -b 115200

format:
	@find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i
	@clang-format -i *.ino

# see https://arduino.github.io/arduino-cli/1.0/getting-started/
install:	check_install
	arduino-cli config init --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json --overwrite
	arduino-cli config set network.connection_timeout 600s
	arduino-cli core update-index
	arduino-cli core install esp32:esp32@3.2.0
	arduino-cli lib install ArduinoJson
	arduino-cli config set library.enable_unsafe_install true
	arduino-cli lib install --git-url https://github.com/ESP32Async/AsyncTCP.git
	arduino-cli lib install --git-url https://github.com/ESP32Async/ESPAsyncWebServer.git
	arduino-cli lib install --git-url https://github.com/devyte/ESPAsyncDNSServer.git

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

