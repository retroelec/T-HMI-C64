# C64 Emulator for the development board Lilygo T-HMI (T-HMI-C64)

C64 emulator for the development board Lilygo T-HMI equipped with an ESP32-S3 chip, a 2.8 inch touch display LCD screen (ST7789V driver) and a SD card slot.

From [Xinyuan-LilyGO/T-HMI](https://github.com/Xinyuan-LilyGO/T-HMI):

<img src="doc/T-HMI.jpg" alt="T-HMI" width="600"/>


## Hardware

### ESP32-S3

The ESP32-S3 is dual core containing a Protocol CPU (known as CPU 0, core 0 or PRO_CPU) and an Application CPU (known as CPU 1, core 1 or APP_CPU).
The two cores are identical in practice and share the same memory.
The tasks responsible for handling wireless networking (Wi-Fi or Bluetooth) are pinned to CPU 0 by default
(see [Espressif - Task Priorities](https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-guides/performance/speed.html)).

For this project core 0 is used to emulate the 6510 CPU and to copy graphic data to a bitmap.
All other application tasks (CIA timers, copy graphic bitmap to LCD) are done on core 1.

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

## Usage

### Arduino Setup

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

### Configure and upload emulator

If desired adapt the following constants in src/Config.h for BLE access:

- SERVICE_UUID
- CHARACTERISTIC_UUID

If you do not use an iduino joystick or choose to use other pins on the development board,
you may have to adapt the following constants in src/Config.h:

- ADC_JOYSTICK_X
- ADC_JOYSTICK_Y
- JOYSTICK_FIRE_PIN

Then just open the file T-HMI-C64.ino in Arduino and upload the emulator (menu Sketch - Upload or press ctrl-u).

### BLE Connect to T-HMI

After uploading the emulator to the T-HMI development board, the C64 startup screen appears.
The emulator starts also a BLE (Bluetooth Low Energy) server to receive keystrokes from a client.
For linux I wrote a small python script named espkb.py (not working under MS Windows). It has to be started in a terminal window.
It first connects to the BLE server of the ESP32. After that it sends keystrokes typed in the terminal window to the emulator.

Before starting espkb.py you have to set the variable device_address to the mac address of the ESP32.
If you did adapt the CHARACTERISTIC_UUID in src/Config.h you also have to set the variable characteristic_uuid in espkb.py accordingly.
You need super user access rights to start the espkb.py script.

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

### Load and start a game

You first have to copy C64 games in prg format (only supported format!) to an SD card
(game names must be in lower case letters, max. 16 characters).

As there is no C64 tape/disk drive emulation available up to now, the file must be loaded
into memory using the external command "load prg from sdcard".
To do this, you first type in the name of the game so it shows up on the C64 text screen.
You then press F12 to toggle to the external mode (if necessary).
You then press F2 to load the game. Use the cursor key "down" to go to the next line and type "run"
to start the game.
Pressing F9 the first time activates joystick in port 1 (joystick mode 1), pressing F9 again activates joystick in port 2 (joystick mode 2)
and pressing F9 again deactivates the joystick (joystick mode 0).

If a game needs keyboard input(s) to be started / configured (e.g. you have to press F1 to start Donkey Kong),
you eventually have to toggle to c64 mode (by pressing F12) AND you have to disable the joysticks
(if already enabled, by pressing F9 until joystick mode 0 is choosen resp. joystick is diabled).
After the start the joystick must be enabled (again) by pressing F9.
(See also point "joystick and keyboard input cannot be applied "at the same time": joystick must be switched off to use keyboard"
in the "Emulation status list".)

## Software

### Class diagram

<img src="doc/classdiagram.png" alt="class diagram" width="600"/>

### Keyboard

Keyboard inputs are sent to the ESP32 via BLE. Three bytes must be sent for each key press:

- Value for the $DC00 register
- Value for the $DC01 register
- Control code:
  - Bit 0 is set when the "left shift" key is pressed
  - Bit 5 is set for the emulation of a joystick in Port 1
  - Bit 6 is set for the emulation of a joystick in Port 2
  - Bit 7 is set when an "External Command" is sent

On the ESP32, a BLE server receives the codes and store them in a buffer.
Every 16.66 ms the three codes are read from the buffer using the getKBCode() method in the "interrupt kernel function" and stored in BLEKB.

When reading the memory location $DC01, the content of the $DC00 memory location is compared with the value sent via BLE for $DC00.
If they match, the value sent via BLE for $DC01 is returned (using the decode() method). This process reflects the procedure of the corresponding kernel routine,
which writes a specific value to the $DC00 register and then reads the $DC01 register to determine the pressed key.

Simulated joystick inputs must first be enabled via a specific external command, determining which joystick port is activated.
The cursor keys and the left control key are then interpreted as joystick inputs.
For joystick inputs from Port 1, the $DC01 register is read, returning the corresponding codes.
For joystick inputs from Port 0, the $DC00 register is read, delivering joystick inputs from Port 2 accordingly when accessing this register.
Joystick simulation using the keyboard is not good enough to play games, but it can be used, e.g., to start a game by pressing the control key which
simulates the fire button of a joystick (if no real joystick is available).

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

### CPU emulation

The emulation of the C64 CPU is sometimes faster than the 1 MHz of a real C64. A simple throtteling mechanism has been implemented to 
reduce the performance of the CPU in these cases.

### VIC emulation

The image is built line by line in a timer interrupt (every 64us), transforming C64 data into a 16-bit color array.
Sprite data is also drawn during this process.
After 200 rows are written to the 16-bit color array, the processed data is transferred to the LCD in the main loop.
I implemented two different approaches to draw the lines:

- approach 1: draw each line one after another
- approach 2: draw even and odd lines successively

For approach 1 a frame rate of 28-29 fps and for approach 2 a frame rate of 25 fps is achived.
These frame rates are enough to run games smoothly.
For most games approach 2 delivers better results
(e.g. Donkey Kong: barely flickering of sprites with approach 2, but with approach 1 the ape sprites flicker sometimes).
Therefore approach 2 is the default, but may be changed using an external command.
Possible reason: Approach 2 allows most of the tasks (data transformation for the current line + eventually triggering a raster line interrupt)
to be completed in the corresponding ESP timer interrupt before the next one starts.

### Emulation status

First of all: This is a hobby project :)

All hardware ports not explicitly mentioned including their corresponding registers are not emulated (e.g. user port and serial port).

"Software stuff" not emulated resp. poorly emulated (list probably not conclusive):

- no SID emulation (and no plans to do this)
- no tape/disk drive emulation
- keyboard emulation using BLE has some problems:
  keyboard is slow, sometime a keystroke is missed, cursor left does not work, ...
- joystick and keyboard input cannot be applied "at the same time": joystick must be switched off to use keyboard
- "lightpen" is not supported (could be simulated using the touch screen of the LCD, but no plans to do this)
- some VIC registers are not implemented (yet): $d01b,  $d01d
- some VIC registers are only partly implemented (yet): $d011 (bit 3+4), $d016 (bit 3)
- no "clipping" of sprites in x direction: they are only visible, if they are completely visible
- some CIA registers are not implemented (yet): $d[c|d]02, $d[c|d]03, $d[c|d]08, $d[c|d]09, $d[c|d]0a, $d[c|d]0b,
- some CIA registers are only partly implemented (yet): $dc0d (bit 2), $dc0e (bit 7), $dc0f (bit 7)
- not all "illegal" opcodes of the 6502 CPU are implemented yet
- line 200 is not always displayed correctly
- code cleanup is necessary
- reduce flickering for some games (appear presumably due to raster interrupts)

Additionally a better BLE client, which works on all platforms, would be nice.

### Games

As there is no disk drive emulation, only "single-load games" can be played.
Up to now I only tested a few games.

Games that are playable:

- wizard of wor
- skramble (from anirog)
- galaxy
- burnin rubber
- lode runner
- donkey kong (rarly crashes)
- bubble bobble
- castle terror
- bagitman
- krakout (only "bat is on the" option can be changed at start screen)

<img src="doc/donkey_kong.png" alt="class diagram" width="600"/>

Some other games run on the emulator, but screen is flickering (badly):

- miner 2049er (still playable)
- dig dug (still playable)
- quartet (still playable)
- international soccer (only mild flickering, playable)
- boulder dash
- tapper
- choplifter

Other games are not working (at the moment):

- q*bert (very slow at start, crashing as game ends)
- hero (slow, really bad flickering)
- fort apocalypse (bad flickering, loosing lifes due to this flickering)
- burger time (crashing)
- ghost and gobblins (crashing)
- great gianas sister (crashing)
- hyper sports (crashing)
- pole position (very slow at start, bad flickering)

