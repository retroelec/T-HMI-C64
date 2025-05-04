ALLBOARDS := T_HMI T_DISPLAY_S3 WAVESHARE
BOARD := T_HMI
#BOARD := T_DISPLAY_S3
#BOARD := WAVESHARE

PORT := /dev/ttyACM0

ifeq ($(BOARD), WAVESHARE)
  FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,DFUOnBoot=dfu,FlashSize=16M,JTAGAdapter=builtin,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,DebugLevel=info
else
  FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,DFUOnBoot=dfu,FlashSize=16M,JTAGAdapter=builtin,PartitionScheme=huge_app,PSRAM=opi,DebugLevel=info
endif

SOURCEFILES=$(wildcard src/*.cpp src/rm67162/*.cpp src/st7789vserial/*.cpp)

default:	T-HMI-C64.ino $(SOURCEFILES) src/loadactions.h src/saveactions.h src/listactions.h
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

compile:	default

compileAll:
	@for board in $(ALLBOARDS); do \
		echo "\ncompiling for $$board"; \
		$(MAKE) compile BOARD=$$board || exit $$?; \
	done

# first you have to get the docker image:
# podman pull docker.io/retroelec42/arduino-cli:latest
podcompile:	T-HMI-C64.ino $(SOURCEFILES) src/loadactions.h src/saveactions.h src/listactions.h
	podman run -it --rm -v .:/workspace/T-HMI-C64 arduino-cli compile --fqbn $(FQBN) --build-property "build.extra_flags=-DBOARD_$(BOARD)" --build-path build$(BOARD) T-HMI-C64.ino

upload:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) -i build$(BOARD)/T-HMI-C64.ino.bin

# create file $HOME/.minirc.dfl 
# content:
# pu addcarreturn    Yes
monitor:
	minicom -D $(PORT) -b 115200

format:
	@clang-format -i *.ino src/*.cpp src/*.h

# see https://arduino.github.io/arduino-cli/1.0/getting-started/
install:	check_install
	arduino-cli config init --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json --overwrite
	arduino-cli core update-index
	arduino-cli core install esp32:esp32@3.0.5

check_install:
	@echo -n "Are you sure? [y/N] " && read ans && [ $${ans:-N} = y ]

