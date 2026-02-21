@echo off
setlocal enabledelayedexpansion

echo --- T-HMI-C64 Flasher (esptool) ---

:: Check: python
python --version >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: 'python' not found. Please install Python.
    pause
    exit /b 1
)

:: Check/Install esptool
python -m esptool version >nul 2>nul
if %errorlevel% neq 0 (
    echo 'esptool' not found. Trying to install it via pip...
    python -m pip install esptool
)

echo Searching for COM ports...
python -m esptool list_ports
echo.

set /p port="Please enter COM-Port (e.g. COM3): "

echo.
echo Flashing merged binary to %port%...
python -m esptool --chip __CHIP__ --port %port% --baud 921600 write_flash 0x0 T-HMI-C64.merged.bin

if %errorlevel% equ 0 (
    echo.
    echo SUCCESS: Firmware flashed successfully!
) else (
    echo.
    echo ERROR: Flash failed.
)

pause
