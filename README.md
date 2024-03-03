# C64 Emulator for the development board Lilygo T-HMI (T-HMI-C64)

C64 emulator for the development board Lilygo T-HMI equipped with an ESP32-S3 chip, a 2.8 inch touch display LCD screen (ST7789V driver) and a SD card slot.

<img src="doc/donkey_kong.png" alt="class diagram" width="600"/>

## Hardware

From [Xinyuan-LilyGO/T-HMI](https://github.com/Xinyuan-LilyGO/T-HMI):

<img src="doc/T-HMI.jpg" alt="T-HMI" width="600"/>

### ESP32-S3

The ESP32-S3 is dual core containing a Protocol CPU (known as CPU 0, core 0 or PRO_CPU) and an Application CPU (known as CPU 1, core 1 or APP_CPU).
The two cores are identical in practice and share the same memory.
The tasks responsible for handling wireless networking (Wi-Fi or Bluetooth) are pinned to CPU 0 by default
(see [Espressif - Task Priorities](https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-guides/performance/speed.html)).

Actually core 0 is used to emulate the 6510 CPU and the VIC and core 1 is used to emulate CIA timers,
copy graphic bitmap to LCD and handling "external commands".

### Display

The 2.8 inch ST7789V TFT LCD has a resolution of 240x320 pixel and an 8 bit parallel interface.

The display can be rotated to support the resolution of a C64 (320x200).

### Joystick

I connected an "iduino joystick shield" to the T-HMI development board.
It has an analog 2-axis thumb joystick and several buttons.

<img src="doc/joystick.png" alt="joystick" width="400"/>

Connections:

- connect T-HMI IO16 to iduino Y pin (yellow cable)
- connect T-HMI IO15 to iduino X pin (white cable)
- connect T-HMI VDD to iduino V pin (red cable)
- connect T-HMI GND to iduino G pin (black cable)
- connect T-HMI IO18 to iduino D pin (for D button, yellow cable)
  
Switch voltage to 3.3V on the iduino module.

If you do not use an iduino joystick or choose to use other pins on the development board,
you may have to adapt the following constants in src/Config.h:

- ADC_JOYSTICK_X
- ADC_JOYSTICK_Y
- JOYSTICK_FIRE_PIN

## Usage

### Files

- T-HMI-C64.ino : Arduino .ino file of the C64 emulator, use in the Arduino IDE to upload the emulator to the T-HMI
- src/* : C64 emulator source code
- THMIC64KB/thmic64kb.apk : Android APK file to be uploaded to your Android smartphone
- THMIC64KB/app/src/ : source code of Android app

### Arduino setup

From [Xinyuan-LilyGO/T-HMI](https://github.com/Xinyuan-LilyGO/T-HMI):
In Arduino Preferences, on the Settings tab, enter the [Espressif Arduino ESP32 Package](https://espressif.github.io/arduino-esp32/package_esp32_index.json)
URL in the Additional boards manager URLs input box.
Click OK and the software will install.
Search for ESP32 in Tools → Board Manager and install ESP32-Arduino SDK (V 2.0.5 or above and below V3.0).

I used the following settings in the Tools menu of the Arduino IDE 2.2.1:

| Setting                              | Value                             |
|--------------------------------------|-----------------------------------|
| Board                                | LilyGo T-Display-S3               |
| Port                                 | /dev/ttyACM0                      |
| USB CDC On Boot                      | Enabled                           |
| Core Debug Level                     | None                              |
| USB DFU On Boot                      | Enabled                           |
| Erase all Flash before sketch upload | Disabled                          |
| Events Run On                        | Core 1                            |
| JTAG Adapter                         | Integrated USB JTAG               |
| Arduino Runs On                      | Core 1                            |
| USB Firmware MSC On Boot             | Disabled                          |
| Partition Scheme                     | 16M Flash (3MB No OTA/1MB SPIFFS) |
| Upload mode                          | UART0 / Hardware CDC              |
| USB Mode                             | Hardware CDC and JTAG             |

### Upload emulator

Just open the file T-HMI-C64.ino in Arduino and upload the emulator (menu Sketch - Upload or press ctrl-u).

### BLE Connection

After uploading the emulator to the T-HMI development board, the C64 startup screen appears.
The emulator starts also a BLE (Bluetooth Low Energy) server to receive keystrokes from a client.
(The needed UUIDs are defined in src/Config.h: SERVICE_UUID and CHARACTERISTIC_UUID.)
Actually there are two clients available.

#### Linux client espkb.py

For linux I wrote a small python script named espkb.py (not working under MS Windows). It has to be started in a terminal window.
It first connects to the BLE server of the ESP32. After that it sends keystrokes typed in the terminal window to the emulator.
Before starting espkb.py you have to set the variable device_address to the mac address of the ESP32.
(If you did adapt the CHARACTERISTIC_UUID in src/Config.h you also have to set the variable characteristic_uuid in espkb.py accordingly.)
You need super user access rights to start the espkb.py script.
To use the script you have to install additional python packages (pygatt, pynput).

Besides sending "normal" C64 keystrokes, it is also possible to send "commands" to the emulator.
The following "external commands" are available:

- f12: toggle between external and c64 mode for f1-f8 (default: c64 mode)
- f9: set joystick
- f1: show cpu + chip registers
- f2: load prg from sdcard
- f3: show memory
- f4: send prg to esp by BLE
- f5: set keyboard joystick
- f6: reset C64
- f7: toggle between 'draw each line one after another' and 'draw even and odd lines successively'

Unfortunatelly the BLE connection is not very stable, keys are transfered slowly and sometimes a keystroke is missed.
So it is discouraged to use this client.

#### Android client

I wrote a simple Android app which emulates a C64 keyboard for the emulator.
You can either install the app using an Android IDE or directly install the APK file on your Android smartphone.

<img src="doc/THMIC64KB.png" alt="THMIC64KB" width="600"/>

Once the app is installed and launched, you must accept the requested permissions
(access to the precise location (*not* coarse location), permission to search for BLE devices).
If you start the emulator (i.e. power on the T-HMI) before starting the app, the app will automatically connect to the BLE server.
Otherwise you can move the "BLE connection" switch to the right to connect to the BLE server. You also have to do this manually
after reseting the development board (e.g. if you want to start a new game).

Besides the normal C64 keys this virtual keyboard also provides red extra buttons to send "external commands".
Actually the LOAD and the JOYSTICK buttons are available:

- LOAD: load a C64 program from SD card
- JOYSTICK: toggle between "joystick in port 1", "joystick in port 2", "no joystick"

Up to now the following keys are not implemented: Commodore key, CTRL key, RESTORE key

BLE connection is stable, fast and reliable - you should use this BLE client.

### Load and start a game

You first have to copy C64 games in prg format (only supported format!) to an SD card
(game names must be in lower case letters, max. 16 characters).

As there is no C64 tape/disk drive emulation available up to now, the file must be loaded
into memory using an "external command".
To do this, you first type in the name of the game so it shows up on the C64 text screen.
You then press the LOAD button on your Android phone.
Use the button RETURN or "cursor key down" to go to the next line
and type "RUN" followed by pressing the button RETURN to start the game.
To use the joystick you have to enable / determine the port of the joystick:
Clicking the JOYSTICK button the first time activates joystick in port 1,
clicking the JOYSTICK button again activates joystick in port 2,
clicking the JOYSTICK button again disables the joystick.

## Software

### Class diagram

<img src="doc/classdiagram.png" alt="class diagram" width="600"/>

### Keyboard

Keyboard inputs are sent to the ESP32 via BLE. Three bytes must be sent for each key press:

- Value for the $DC00 register
- Value for the $DC01 register
- Control code:
  - Bit 0 is set when a shift key is pressed
  - Bit 7 is set when an "external command" is sent

### Memory synchronization

Some variables are accessed by both cores. These variables must be synchronized.

From the ESP32-S3 documentation to the issue "potential cache data coherence":  
There are three common methods to address such cache data coherence issue:

- Hardware based cache Coherent Interconnect, ESP32-S3 does not have such ability.
- Use the DMA buffer from non-cacheable memory. Memory that CPU access it without going through cache is called non-cacheable memory.
- Explicitly call a memory synchronization API to writeback the content in the cache back to the memory, or invalidate the content in the cache.

As there is no hardware based solution to this problem, I choosed to rely on the standard C++ synchronization mechanisms, i.e. using atomic variables.

To ensure that changes to a std::atomic variable are visible between different processor cores and cache inconsistencies are avoided,
memory_order_release for writing and memory_order_acquire for reading can be used as a synchronization mechanism.

### Emulation details

Emulation of CPU and VIC (on core 0) are tightly coupled: After 63 CPU cycles the next raster line is handeled. This procedure is necessary to run games
which use raster line interrupts. However, by default only each second row is copied to a 16-bit color array per frame
(changing between even and odd rows each frame), otherwise the CPU performance would be slowed down noticeable.
Sprite data is also drawn during this process.

On core 1 the data of the 16-bit color array is copied to the LCD screen. A frame rate of about 29 fps is achived.

### Emulation status

First of all: This is a hobby project :)

All hardware ports not explicitly mentioned including their corresponding registers are not emulated (e.g. user port and serial port).

"Software stuff" not emulated resp. poorly emulated resp. things to do resp. known bugs (list probably not conclusive):

- no SID emulation (and no plans to do this)
- no tape/disk drive emulation
- Android app: implement Commodore key, CTRL key, RESTORE key
- some VIC registers are not implemented (yet): $d01b
- some VIC registers are only partly implemented (yet): $d011 (bit 3+4), $d016 (bit 3)
- no "clipping" of sprites in x direction: they are only visible if they are completely visible
- some CIA registers are not implemented (yet): $d[c|d]02, $d[c|d]03, $d[c|d]08, $d[c|d]09, $d[c|d]0a, $d[c|d]0b,
- some CIA registers are only partly implemented (yet): $dc0d (bit 2), $dc0e (bit 7), $dc0f (bit 7)
- not all "illegal" opcodes of the 6502 CPU are implemented yet
- line 200 is not displayed correctly
- code cleanup is necessary
- sometimes CPU is blocked after loading a game
- no joystick emulation using BLE
- some games are not running properly
- some games are not working at all

### Games

As there is no disk drive emulation, only "single-load games" can be played.
Up to now I only tested a few games.

Games that are playable:

- wizard of wor
- skramble (from anirog)
- galaxy
- burnin rubber
- lode runner
- donkey kong
- bubble bobble
- castle terror
- bagitman
- krakout (only "bat is on the" option can be changed at start screen)
- miner 2049er
- dig dug (background is mildly flickering)
- quartet
- international soccer
- choplifter
- pole position

Games not running properly:

- hero (sprites flickering, magnetic walls are too dark(?), still playable)
- boulder dash (sprites flickering, wrong colors)
- q*bert (sprites flickering, graphic errors after game over)
- fort apocalypse (sprites flickering, sometimes player sprite disappears)

Games not working at all:

- burger time (crashing)
- ghost and gobblins (crashing)
- great gianas sister (crashing)
- hyper sports (crashing)

