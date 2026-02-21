#!/bin/bash

echo "--- T-HMI-C64 Flasher (esptool) ---"

# 1. Check: python3
if ! command -v python3 &> /dev/null; then
    echo "ERROR: 'python3' not found. Please install Python."
    exit 1
fi

# 2. Check/Install esptool
if ! python3 -m esptool version &> /dev/null; then
    echo "esptool not found. Installing..."
    python3 -m pip install esptool --break-system-packages || python3 -m pip install esptool
fi

echo "Connected ports:"
python3 -m esptool list_ports
echo ""

# Port detection
DEFAULT_PORT="/dev/ttyACM0"
if [ "$(uname -s)" = "Darwin" ]; then
    DEFAULT_PORT="/dev/tty.usbmodem1101"
fi

read -p "Enter port [$DEFAULT_PORT]: " PORT
PORT=${PORT:-$DEFAULT_PORT}

echo "Flashing merged binary to $PORT..."
python3 -m esptool --chip __CHIP__ --port "$PORT" --baud 921600 write_flash 0x0 T-HMI-C64.merged.bin

if [ $? -eq 0 ]; then
    echo -e "\nSUCCESS: Firmware flashed successfully!"
else
    echo -e "\nERROR: Flash failed."
    exit 1
fi
