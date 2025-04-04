# C64 Emulator for ESP32-S3 with "Android keyboard" (BLE) for Lilygo T-HMI and T-Display S3 AMOLED

C64 emulator for the development board Lilygo T-HMI equipped with an ESP32-S3 chip, a 2.8 inch touch display LCD screen (ST7789V driver) and an SD card slot.
The emulator was later expanded to also support the board T-Display S3 AMOLED.

The keyboard for the emulator is simulated by an Android app, communication between the app and the emulator is realized using BLE (Bluetooth Low Energy).

[![C64 Emulator on development board Lilygo T-HMI](doc/donkey_kong.png)](https://youtu.be/OmPJlIjszpE)

Contact: retroelec42@gmail.com

## News

- Sending a file to the development board by BLE
- Improved keyboard handling (optional detection of releasing a key)
- Operating the T-HMI via battery works now

## Lilygo T-HMI development board

From [Xinyuan-LilyGO/T-HMI](https://github.com/Xinyuan-LilyGO/T-HMI):

<img src="doc/T-HMI.jpg" alt="T-HMI" width="800"/>

### ESP32-S3

The ESP32-S3 is dual core containing a Protocol CPU (known as CPU 0, core 0 or PRO_CPU) and an Application CPU (known as CPU 1, core 1 or APP_CPU).
The two cores are identical in practice and share the same memory.
The tasks responsible for handling wireless networking (Wi-Fi or Bluetooth) are pinned to core 0 by default
(see [Espressif - Task Priorities](https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-guides/performance/speed.html)).

For this project core 0 is used to copy the graphic bitmap to LCD.
Emulation of the CPU and the custom chips (VIC and CIAs) are done on core 1.

### Display

The 2.8 inch ST7789V TFT LCD has a resolution of 240x320 pixel and an 8 bit parallel interface.

The display can be rotated to support the resolution of a C64 (320x200).

### Joystick

I connected an "Arduino joystick shield" to the T-HMI development board.
The joystick is optional (as there exists also a virtual joystick on the Android device) but recommended.
It has an analog 2-axis thumb joystick and several buttons.
As there are several games which use the space bar as a second fire button (e.g. Commando), another button of the Arduino joystick
can be used to simulate the pressing of the space bar.

<img src="doc/joystick.png" alt="joystick" width="800"/>

Connections:

- connect T-HMI IO16 to Arduino joystick Y pin (yellow cable)
- connect T-HMI IO15 to Arduino joystick X pin (white cable)
- connect T-HMI VDD to Arduino joystick V pin (red cable)
- connect T-HMI GND to Arduino joystick G pin (black cable)
- connect T-HMI IO18 to Arduino joystick D pin (for D button, yellow cable)
- optional: connect T-HMI IO17 to Arduino joystick B pin to simulate pressing the space bar (for B button, white cable)

Switch voltage to 3.3V on the Arduino joystick module.

If you do not use an Arduino joystick or choose to use other pins on the development board,
you may have to adapt the following constants in src/Config.h:

- ADC_JOYSTICK_X
- ADC_JOYSTICK_Y
- JOYSTICK_FIRE_PIN
- JOYSTICK_FIRE2_PIN

### Battery

You can also operate your board with a battery. The T-HMI development board allows you to switch on the board by pressing
the On/Off switch (next to the SD card slot). However, it does not appear to be possible to turn the board off with this switch.
Therefore, the Android app allows the board to be switched off by pressing the Off switch in the top right-hand corner.

## Installation

### Files

- build/*.bin : Binary files of the C64 emulator to be uploaded to the T-HMI
- THMIC64KB/thmic64kb.apk : Android APK file to be uploaded to your Android smartphone
- T-HMI-C64.ino : Arduino .ino file of the C64 emulator, use in the Arduino IDE to upload the emulator to the T-HMI
- src/* : C64 emulator source code
- THMIC64KB/app/src/ : source code of Android app
- Makefile : used to install development environment and to compile + upload code

### Install C64 Emulator on the Lilygo T-HMI board

I use arduino-cli to upload the provided binary files to the development board:

- Download arduino-cli for your platform (e.g. https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Linux_64bit.tar.gz for linux),
  unpack the binary and place it in a directory included in the search path of executables (e.g. /usr/local/bin on linux).
- You may have to install python3 and python3-serial if not already installed. On my linux system I had to install python3-serial:  
  sudo apt install python3-serial
- You may have to install esptool to flash the microcontroller if not already installed:  
  pip install esptool
- Execute arduino-cli with the correct parameters (you may have to replace "/dev/ttyACM0" with the name of the serial port on your system):  
  arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,DFUOnBoot=dfu,FlashSize=16M,JTAGAdapter=builtin,PartitionScheme=huge_app,PSRAM=opi,LoopCore=0,DebugLevel=info -i build/T-HMI-C64.ino.bin

If you want to install the development environment, you can use the provided Makefile (which itself uses arduino-cli) or you can use the Arduino IDE.

#### Install development environment using Makefile (*not* necessary to run the emulator)

- Download arduino-cli for your platform (https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Linux_64bit.tar.gz),
  unpack the binary and place it in a directory included in the search path of executables (e.g. /usr/local/bin on a linux system).
- You may have to install GNU make if not already installed.
- Install required Arduino core and libraries using the following command in the directory T-HMI-C64:  
  make install
- You may have to install python3 and python3-serial if not already installed. On my linux system I had to install python3-serial:  
  sudo apt install python3-serial
- You may have to install the esptool to flash your microcontroller if not already installed:  
  pip install esptool
- You may have to adapt the file Makefile and change the name of the serial port (adapt variable PORT).
- On a linux system you may have to add the group dialout to your user to be able to upload code as a normal user:  
  sudo usermod -a -G dialout your-username  
  (You have to logout and login again to get the group get active.)
- Compile code (see also chapter "Compiling code without changing the actual version of your installed Arduino core"):  
  make
- Upload code:  
  make upload

##### Compiling code without changing the actual version of your installed Arduino core

You may have installed already a specfic Arduino core for other projects (e.g. Arduino core 2.0.x) and you don't want to change this setup.
For this situation you can use a prepared docker image to compile the code:

- install podman
- get prepared docker image: podman pull docker.io/retroelec42/arduino-cli:latest
- compile using docker image: make podcompile

#### Install development environment using Arduino IDE (*not* necessary to run the emulator)

From [Xinyuan-LilyGO/T-HMI](https://github.com/Xinyuan-LilyGO/T-HMI):
In Arduino Preferences, on the Settings tab, enter the [Espressif Arduino ESP32 Package](https://espressif.github.io/arduino-esp32/package_esp32_index.json)
URL in the Additional boards manager URLs input box.
Click OK and the software will install.
Search for ESP32 in Tools → Board Manager and install ESP32-Arduino SDK (V3.0.5).

Use the following settings in the Tools menu of the Arduino IDE 2.3.2:

| Setting                              | Value                             |
|--------------------------------------|-----------------------------------|
| Board                                | ESP32 S3 Dev Module               |
| Port                                 | /dev/ttyACM0                      |
| USB CDC On Boot                      | Enabled                           |
| CPU Frequency                        | 240MHz (WiFi)                     |
| Core Debug Level                     | None                              |
| USB DFU On Boot                      | Enabled                           |
| Events Run On                        | Core 1                            |
| Flash Mode                           | QIO 80 MHz                        |
| Flash Size                           | 16MB (128Mb)                      |
| JTAG Adapter                         | Integrated USB JTAG               |
| Arduino Runs On                      | Core 0                            |
| USB Firmware MSC On Boot             | Disabled                          |
| Partition Scheme                     | Huge APP (3MB No OTA/1MB SPIFFS)  |
| Upload mode                          | UART0 / Hardware CDC              |
| PSRAM                                | OPI PSRAM                         |
| USB Mode                             | Hardware CDC and JTAG             |
| Core Debug Level                     | Info                              |

The following Arduino libraries are used (all are part of ESP32 Arduino core, version 3.0):

- FS
- SD_MMC
- ESP32 BLE Arduino

To upload the emulator from the Arduino IDE just open the file T-HMI-C64.ino
and choose menu Sketch - Upload or press ctrl-u.

### Install C64 Emulator on the T-Display S3 AMOLED board

Provided files (source code and binary files) are ready for the board T-HMI.
If you want to install the emulator on the T-Display S3 AMOLED board, you have to adapt src/Config.h:

comment line "#define BOARD_T_HMI"  
uncomment line "//#define BOARD_T_DISPLAY_S3"  

Afterwards you have to compile and upload the code to the T-Display S3 AMOLED board (see chapter "Install C64 Emulator on Lilygo T-HMI").

Note: Files rm67162.cpp, rm67162.h and pins_config.h were copied from https://github.com/Xinyuan-LilyGO/T-Display-S3-AMOLED/tree/main/examples/factory.

As the board has no SD card you can send a programm from your Android device to the emulator (see below).

<img src="doc/tdisps3amoled.jpg" alt="T-HMI" width="800"/>

### Install Android App

I wrote an Android app which provides a C64 keyboard for the emulator.

<img src="doc/THMIC64KB.png" alt="THMIC64KB" width="1000"/>

However, this app is not available in the Google Play Store - you have to download the APK file
and install it "manually".
You may follow these steps to install the app on your Android device (there may be slight variations depending on your smartphone).

1. Allow installation of APK files from unknown sources:
   - Go to "Settings" on your Android device.
   - Navigate to "Security and Privacy".
   - Navigate to "Additional Security Settings".
   - Navigate to "Install Unknown Apps". A list of installed apps appears. Allow Chrome to install unknown apps.
2. Download the APK file to your Android device: Click [here](https://github.com/retroelec/T-HMI-C64/blob/main/THMIC64KB/thmic64kb.apk)
3. After the app has been downloaded, a message appears which allows you to open the file.
   Click on open and follow the on-screen instructions to complete the installation.

## Usage

### Android keyboard

The emulator starts a BLE (Bluetooth Low Energy) server to receive keystrokes from the Android client.

Once the app is installed and launched, you must accept the requested permissions once
(access to the precise location (*not* coarse location), permission to search for BLE devices).
If you start the emulator (i.e. power on the T-HMI) before starting the app, the app will automatically connect to the BLE server.
Otherwise you can move the "BLE connection" switch to the right to connect to the BLE server. You also have to do this manually
after "hardware reseting" the development board.

As it is not possible to press two keys together on the Android keyboard, the keys Shift, Ctrl and Commodore are special keys
which usually are pressed first, followed by another key to simulate the corresponding key combination.

If it is necessary to send the raw key code of these special keys, you have to
set the corresponding switch in the Android app ("Send raw keycodes", DIV screen).

By default, only the pressing of a key is sent to the emulator, which results in, for example, only one space being output
even if the space key is held down. With the 'Detect release key' option (DIV screen), the C64 keyboard is emulated more authentically
(with the drawback that typing speed is reduced).

The key combination Run/Stop + Restore has been replaced by first pressing the Commodore key and then pressing the Restore key.

Besides the normal C64 keys this virtual keyboard also provides some extra buttons:

- DIV: opens an extra screen with some settings / extra functionality
- KBINP: opens an extra screen with further input options ("virtual joysticks" and "pinball flippers")
- JOY1: connected joystick can be used as a joystick in port 1
- JOY2: connected joystick can be used as a joystick in port 2
- LOAD: load a C64 program from SD card
- RESET: reset C64
- OFF: switch off T-HMI development board

#### DIV screen

<img src="doc/THMIC64KB_Div.png" alt="DIV Screen" width="800"/>

Extra functionality and some settings are available in the DIV screen.

#### Virtual Joystick

<img src="doc/THMIC64KB_VirtJoystick.png" alt="Virtual Joystick" width="800"/>

The virtual joystick has some drawbacks in terms of responsiveness.
To play games, a hardware joystick is recommended.

#### Pinball flippers

<img src="doc/THMIC64KB_Pinball.png" alt="Pinball Flippers" width="800"/>

Because it is difficult to keep an eye on the screen and press the right keys on the virtual keyboard at the same time for pinball games,
the flippers of “David's Midnight Magic” have been outsourced to a separate screen.
When you open this screen, the options "Send raw keycodes" and "Detect key release" are automatically enabled.
When you close this screen again, both options are reset to the previous values.

### Load a program from SD card

You first have to copy C64 games in prg format (only supported format!) to an SD card
(game names must be in lower case letters, max. 16 characters, no spaces in file names allowed, extension must be ".prg", e.g. dkong.prg).
You have to insert the SD card before you power on the T-HMI development board.

As there is no C64 tape/disk drive emulation available, the file must be loaded
into memory using an "external command".
To do this, you first type in the name of the program (without extension ".prg"!) so it shows up on the C64 text screen (e.g. dkong).
You then press the LOAD button on your Android phone (cursor must be on the same line and behind or in the middle of the game title).
If the file is found the text "LOADED" appears on screen, otherwise the text "FILE NOT FOUND" appears.
Afterwards, as usual, you can start the game by typing "RUN" followed by pressing the button RETURN.

<img src="doc/loadprg.gif" alt="class diagram" width="800"/>

Hint: You can use [D64 Editor](https://www.d64editor.com/) to extract prg files from d64 files (also works unter linux using wine).

### Save a program to SD card

Type in the name of the program so that it appears on the screen. Then press the SAVE button on the DIV screen to save the program.

### List programs on SD card

Just press the LIST button on the DIV screen. If more than 23 programs are stored on the SD card,
you can display all programs stored on the SD card by repeatedly pressing the LIST button.

### Send a program by BLE

You can also send a programm from your Android device to the emulator (DIV screen, SENDPRG Button).

## Software

### Class diagram of the emulator

<img src="doc/classdiagram.png" alt="class diagram" width="600"/>

### Keyboard

Keyboard inputs are sent to the ESP32 via BLE. Three bytes must be sent for each key press:

- Value for the $DC00 register
- Value for the $DC01 register
- Control code:
  - Bit 0 is set when a shift key is pressed
  - Bit 1 is set when the ctrl key is pressed
  - Bit 2 is set when the commodore key is pressed
  - Bit 7 is set when an "external command" is sent

### Emulation status

First of all: This is a hobby project :)

All hardware ports not explicitly mentioned including their corresponding registers are not emulated (e.g. user port and serial port).

"Software stuff" not emulated resp. known bugs:

- no SID emulation (no sound support of development board)
- no tape/disk drive emulation (and no plans to do this)
- "illegal instructions" test suite fails
- rarly CPU is blocked after loading a game
- some games have graphic errors
- some games are not working at all

### Games

As there is no disk drive emulation, only "onefiled games" can be played.
Up to now I tested the following games.  
Note: I do not test all games after every patch - it is therefore possible that some games only work with an earlier patch.

Games that are playable:

- Wizard of Wor
- Skramble (from Anirog)
- Galaxy
- Burnin Rubber
- Lode Runner
- Donkey Kong
- Bubble Bobble
- Castle Terror
- Bagitman
- Krakout
- Miner 2049er
- Dig Dug
- Quartet
- International Soccer
- Choplifter
- Pole Position
- Pole Position 2
- Pacman
- Boulder Dash
- Ghost and Gobblins (graphic errors at bottom)
- Great Gianas Sister
- Hyper Sports
- Blue Max
- Commando (graphic errors at bottom)
- Fort Apocalypse
- Hero
- Burger Time 97
- Burger Time 2023
- Outrun (deactivate CIA2 on DIV screen)
- Q*bert
- Space Taxi
- Jupiter Lander
- Dark Caves
- Blobber
- Galaxions
- BC's Quest for Tires
- Stunt Car Racer
- Tapper
- Starforce
- 8Midas
- Acid Runner (wrong colors)
- Gyroscope
- International Karate
- Jumpman
- Little Computer People
- Montezuma's Revenge
- Pooyan
- Track & Field
- Wonder Boy
- Squish'em
- Thing on a spring
- Xevious
- Zalaga
- Zaxxon
- Rambo
- Rick Dangerous
- David's Midnight Magic (hint: use Android screen KBINP/PINBALL1) 
- Terra Cresta (playable, but game is too slow)
- Arkanoid
- Ghostbusters
- Bruce Lee
- 1943 (graphic errors at bottom)
- Nebulus
- Drol
- Paradroid

Games not working:

- Burger Time (crashing)
- Westbank (endless loop)
- 1942 (no proper scrolling)
- Lemmings (incomplete graphics)
- Buggy Boy (endless loop)
- Uridium (crashing)
- Wizball (crashing)

