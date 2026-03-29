# 🕹️ C64 Emulator - Flashing Instructions

This package contains the pre-compiled firmware for your ESP32S3 and a script to flash it easily.

## 📋 Prerequisites

1. **Python 3:** Ensure Python is installed on your system.
2. **Drivers:** Make sure the USB-to-Serial drivers for your board (CH340 or CP210x) are installed.

*The flash script will automatically try to install the required 'esptool' via pip if it's missing.*

---

## 🚀 How to Flash

1. **Connect your device** via USB.
2. **Run the script**:
   * **Windows:** Double-click `flash.bat`
   * **Linux/macOS:** Open a terminal and run `chmod +x flash.sh` followed by `./flash.sh`
3. **Select the Port:**
   * The script will list available COM/Serial ports.
   * Enter your port (e.g., `COM3` or `/dev/ttyACM0`) and press ENTER.
4. **Wait:** The process takes about 20-30 seconds.

---

## 🛠️ Advanced (Manual Flashing)

If you prefer the command line, use:
esptool.py --chip esp32s3 --port [YOUR_PORT] --baud 921600 write_flash -z 0x0 C64Emu.ino.merged.bin

---

## ❓ Troubleshooting

* **Permission Denied (Linux):** Run `chmod +x flash.sh`. You might also need: `sudo usermod -a -G dialout $USER`.
* **Port not found:** Check your USB cable (must be a data cable, not just a charging cable).
* **Connection Error:** If the flash fails to start, enter the upload mode manually:
    * Connect the board via the USB cable
    * Press and hold the BOOT button, while still pressing the BOOT button, press the RESET button
    * Release the RESET button
    * Release the BOOT button
    * Upload C64Emu.ino.merged.bin

