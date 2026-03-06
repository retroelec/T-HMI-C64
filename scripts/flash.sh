#!/bin/bash

BOARD="__BOARD__"
CHIP="__CHIP__"
BINARY="C64Emu.ino.merged.bin"

echo "========================================================="
echo "  T-HMI-C64 Firmware Flasher for $BOARD"
echo "========================================================="

# 1. check python
if ! command -v python3 &> /dev/null; then
    echo "Error: python3 is not installed. Please install Python."
    exit 1
fi

# 2. check/install esptool
if ! python3 -m esptool version &> /dev/null; then
    echo "esptool not found. Installing..."
    python3 -m pip install esptool --break-system-packages || python3 -m pip install esptool
fi

# 3. port detection
echo "Searching for connected USB devices..."
echo "---------------------------------------------------------"
python3 -c "import serial.tools.list_ports; [print(f'{p.device} - {p.description}') for p in serial.tools.list_ports.comports() if p.hwid != 'n/a']"
echo "---------------------------------------------------------"
SUGGESTED_PORT=$(python3 -c "import serial.tools.list_ports; print(next((p.device for p in serial.tools.list_ports.comports() if p.hwid != 'n/a'), ''))")
if [ -n "$SUGGESTED_PORT" ]; then
    read -p "Enter port (Default: $SUGGESTED_PORT): " PORT
    PORT=${PORT:-$SUGGESTED_PORT}
else
    echo "No USB device detected automatically."
    read -p "Enter port (e.g. /dev/ttyACM0): " PORT
fi
if [ -z "$PORT" ]; then
    echo "Error: No port specified."
    exit 1
fi

# 4. flash binaries
echo ""
echo "Flashing $BINARY to $BOARD on port $PORT..."
echo "---------------------------------------------------------"
python3 -m esptool --chip "$CHIP" --port "$PORT" --baud 921600 write_flash -z --flash-mode dio --flash-size keep 0x0 "$BINARY"

if [ $? -eq 0 ]; then
    echo ""
    echo "SUCCESS: Firmware flashed successfully!"
else
    echo ""
    echo "ERROR: Flashing failed."
    echo "Check if the board is in Bootloader Mode (hold BOOT, press RESET)."
    exit 1
fi
