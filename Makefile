default:
	arduino-cli compile --fqbn esp32:esp32:lilygo_t_display_s3 T-HMI-C64.ino

upload:
	arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:lilygo_t_display_s3 T-HMI-C64.ino

monitor:
	minicom -D /dev/ttyACM0 -b 115200

format:
	@clang-format -i *.ino src/*.cpp src/*.h
