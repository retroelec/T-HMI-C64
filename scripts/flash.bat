@echo off
setlocal enabledelayedexpansion

set BOARD=__BOARD__
set CHIP=__CHIP__
set BINARY=C64Emu.ino.merged.bin

echo =========================================================
echo   T-HMI-C64 Firmware Flasher for %BOARD%
echo =========================================================

:: 1. check python
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: Python is not installed or not in PATH.
    echo Please install Python from python.org and check 'Add to PATH'.
    pause
    exit /b 1
)

:: 2. check/install esptool
python -c "import esptool" >nul 2>&1
if %errorlevel% neq 0 (
    echo esptool not found. Installing...
    python -m pip install esptool
)

:: 3. port detection
echo Searching for connected USB COM ports...
echo ---------------------------------------------------------
python -c "import serial.tools.list_ports; [print(f'{p.device} - {p.description}') for p in serial.tools.list_ports.comports() if p.hwid != 'n/a']"
echo ---------------------------------------------------------
for /f "tokens=*" %%i in ('python -c "import serial.tools.list_ports; print(next((p.device for p in serial.tools.list_ports.comports() if p.hwid != 'n/a'), ''))"') do set SUGGESTED_PORT=%%i
if not "%SUGGESTED_PORT%"=="" (
    set /p PORT="Enter port (Default: %SUGGESTED_PORT%): "
    if "!PORT!"=="" set PORT=%SUGGESTED_PORT%
) else (
    echo No USB device detected automatically.
    set /p PORT="Enter port (e.g. COM3): "
)
if "%PORT%"=="" (
    echo Error: No port specified.
    pause
    exit /b 1
)

:: 4. flash binary
echo.
echo Flashing %BINARY% to %BOARD% on port %PORT%...
echo ---------------------------------------------------------

python -m esptool --chip %CHIP% --port %PORT% --baud 921600 write_flash --flash_mode dio --flash_size keep 0x0 %BINARY%

if %errorlevel% equ 0 (
    echo.
    echo SUCCESS: Firmware flashed successfully!
    echo Please reset your %BOARD% now.
) else (
    echo.
    echo ERROR: Flashing failed.
    echo Check if the board is in Bootloader Mode (hold BOOT, press RESET).
)

pause
