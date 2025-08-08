ALLBOARDS := T_HMI T_DISPLAY_S3 WAVESHARE
#BOARD := T_HMI
BOARD := T_DISPLAY_S3
#BOARD := WAVESHARE

PORT := /dev/ttyACM0

ifeq ($(BOARD), WAVESHARE)
  FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,DFUOnBoot=dfu,FlashSize=16M,JTAGAdapter=builtin,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,DebugLevel=info
else
  FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,DFUOnBoot=dfu,FlashSize=16M,JTAGAdapter=builtin,PartitionScheme=huge_app,PSRAM=opi,DebugLevel=info
endif

SOURCEFILES := $(shell find src -name '*.cpp')
HEADERFILES := $(shell find src -name '*.h')

TARGET := build$(BOARD)/T-HMI-C64.ino.elf

$(TARGET):	T-HMI-C64.ino $(SOURCEFILES) $(HEADERFILES)
	arduino-cli compile --warnings all --fqbn $(FQBN) --build-property "build.extra_flags=-DBOARD_$(BOARD)" --build-path build$(BOARD) T-HMI-C64.ino

src/loadactions.h:	src/loadactions.asm
	/opt/TMPx_v1.1.0-STYLE/linux-x86_64/tmpx src/loadactions.asm -o src/loadactions.prg
	xxd -i src/loadactions.prg > src/loadactions.h

src/saveactions.h:	src/saveactions.asm
	/opt/TMPx_v1.1.0-STYLE/linux-x86_64/tmpx src/saveactions.asm -o src/saveactions.prg
	xxd -i src/saveactions.prg > src/saveactions.h

src/listactions.h:	src/listactions.asm
	/opt/TMPx_v1.1.0-STYLE/linux-x86_64/tmpx src/listactions.asm -o src/listactions.prg
	xxd -i src/listactions.prg > src/listactions.h

compile:	$(TARGET)

clean:
	rm -rf build$(BOARD)

compileAll:
	@for board in $(ALLBOARDS); do \
		echo "\ncompiling for $$board"; \
		$(MAKE) clean compile BOARD=$$board || exit $$?; \
	done

# first you have to get the docker image:
# podman pull docker.io/retroelec42/arduino-cli:latest
podcompile:	T-HMI-C64.ino $(SOURCEFILES) src/loadactions.h src/saveactions.h src/listactions.h
	podman run -it --rm -v .:/workspace/T-HMI-C64 arduino-cli-thmic64 compile --fqbn $(FQBN) --build-property "build.extra_flags=-DBOARD_$(BOARD)" --build-path build$(BOARD) T-HMI-C64.ino

upload:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) -i build$(BOARD)/T-HMI-C64.ino.bin

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
	arduino-cli core update-index
	arduino-cli core install esp32:esp32@3.2.0

check_install:
	@echo -n "Are you sure? [y/N] " && read ans && [ $${ans:-N} = y ]

# Linux

SRCDIR := src
BUILDDIRLINUX := buildlinux
TARGETLINUX := c64linux

OBJFILESLINUX := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIRLINUX)/%.o,$(SOURCEFILES))

CXX := g++
CXXFLAGS := -Wall -MMD -MP -DPLATFORM_LINUX -Isrc

$(TARGETLINUX): $(OBJFILESLINUX)
	$(CXX) $(OBJFILESLINUX) -o $@ -lSDL2

$(BUILDDIRLINUX)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJFILESLINUX:.o=.d)

cleanlinux:
	rm -rf $(BUILDDIRLINUX) $(TARGETLINUX)

.PHONY: cleanlinux

