FQBN := esp32:esp32:esp32s3:CDCOnBoot=cdc,DFUOnBoot=dfu,FlashSize=16M,JTAGAdapter=builtin,PartitionScheme=huge_app,PSRAM=opi,LoopCore=0,DebugLevel=info
PORT := /dev/ttyACM0
SOURCEFILES=$(wildcard src/*.cpp)

default:	T-HMI-C64.ino $(SOURCEFILES) src/loadactions.h
	arduino-cli compile --fqbn $(FQBN) T-HMI-C64.ino

src/loadactions.h:	src/loadactions.asm
	/opt/TMPx_v1.1.0-STYLE/linux-x86_64/tmpx src/loadactions.asm -o src/loadactions.prg
	xxd -i src/loadactions.prg > src/loadactions.h

upload:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) T-HMI-C64.ino

# create file $HOME/.minirc.dfl 
# content:
# pu addcarreturn    Yes
monitor:
	minicom -D $(PORT) -b 115200

format:
	@clang-format -i *.ino src/*.cpp src/*.h

# see https://arduino.github.io/arduino-cli/0.28/getting-started/
install:
	arduino-cli config init --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json --overwrite
	arduino-cli core update-index
	arduino-cli core install esp32:esp32
