#!/usr/bin/python3

# Copyright (C) 2024 retroelec <retroelec42@gmail.com>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# For the complete text of the GNU General Public License see
# http://www.gnu.org/licenses/.

import pygatt
from pynput import keyboard
from termios import tcflush, TCIFLUSH
import sys
import re
import subprocess


# GATT charakteristic
characteristic_uuid = "3b05e9bf-086f-4b56-9c37-7b7eeb30b28b"

# mac address of esp32
device_address = "34:85:18:AB:37:71"

# f1-f8: flag to switch between host and c64 (start: f1-f8: host cmds)
f12hostmode = False

# joystick
realjoystick = 0
kbjoystick = 0

# hostmode constants
SETJOYSTICKMODE = 1
SETKBJOYSTICKMODE = 2
LOAD = 11
RECEIVEDATA = 12
SHOWREG = 13
SHOWMEM = 14
RESET = 20
TOGGLEVICDRAW = 21

def get_active_window():
    root = subprocess.Popen(['xprop', '-root', '_NET_ACTIVE_WINDOW'], stdout=subprocess.PIPE)
    stdout, stderr = root.communicate()
    m = re.search(b'^_NET_ACTIVE_WINDOW.* ([\w]+)$', stdout)
    if m != None:
        return m.group(1)
    else:
        return None

# function to read a file into a byte array
def read_file_to_byte_array(file_path):
    try:
        with open(file_path, 'rb') as file:
            byte_array = bytearray(file.read())
        return byte_array
    except FileNotFoundError:
        print(f"file '{file_path}' not found")
    except Exception as e:
        print(f"error reading file: {e}")

# function to send bytes to esp32
def send_key(key):
    device.char_write(characteristic_uuid, key)
    value = device.char_read(characteristic_uuid)

# key pressed handler
def on_press(key):
    global f12hostmode
    global realjoystick
    global kbjoystick
    if activewin != get_active_window():
        return None
    try:
        # print(f'Key {key} pressed')
        # special keys
        if key == keyboard.Key.backspace:
            send_key(bytes([0xfe, 0xfe, 0x00]))
        elif key == keyboard.Key.insert:
            send_key(bytes([0xfe, 0xfe, 0x01]))
        elif key == keyboard.Key.enter:
            send_key(bytes([0xfe, 0xfd, 0x00]))
        elif not f12hostmode and key == keyboard.Key.f7:
            send_key(bytes([0xfe, 0xf7, 0x00]))
        elif not f12hostmode and key == keyboard.Key.f8:
            send_key(bytes([0xfe, 0xf7, 0x01]))
        elif not f12hostmode and key == keyboard.Key.f1:
            send_key(bytes([0xfe, 0xef, 0x00]))
        elif not f12hostmode and key == keyboard.Key.f2:
            send_key(bytes([0xfe, 0xef, 0x01]))
        elif not f12hostmode and key == keyboard.Key.f3:
            send_key(bytes([0xfe, 0xdf, 0x00]))
        elif not f12hostmode and key == keyboard.Key.f4:
            send_key(bytes([0xfe, 0xdf, 0x01]))
        elif not f12hostmode and key == keyboard.Key.f5:
            send_key(bytes([0xfe, 0xbf, 0x00]))
        elif not f12hostmode and key == keyboard.Key.f6:
            send_key(bytes([0xfe, 0xbf, 0x01]))
        elif kbjoystick == 0 and key == keyboard.Key.right:
            send_key(bytes([0xfe, 0xfb, 0x00]))
        elif kbjoystick == 0 and key == keyboard.Key.left:
            send_key(bytes([0xfe, 0xfb, 0x01]))
        elif kbjoystick == 0 and key == keyboard.Key.down:
            send_key(bytes([0xfe, 0x7f, 0x00]))
        elif kbjoystick == 0 and key == keyboard.Key.up:
            send_key(bytes([0xfe, 0x7f, 0x01]))
        # commodore key
        elif kbjoystick == 0 and key == keyboard.Key.ctrl:
            send_key(bytes([0x7f, 0xdf, 0x00]))
        elif key == keyboard.Key.home:
            send_key(bytes([0xbf, 0xf7, 0x00]))
        elif key == keyboard.Key.end:
            send_key(bytes([0xbf, 0xf7, 0x01]))
        elif key == keyboard.Key.space:
            send_key(bytes([0x7f, 0xef, 0x00]))
        elif key == keyboard.Key.esc:
            send_key(bytes([0x7f, 0x7f, 0x00]))
        # toggle between host and c64 mode for f1-f8
        elif key == keyboard.Key.f12:
            f12hostmode = not f12hostmode
            print(f"hostmode = {f12hostmode}")
        # load from sd card
        elif f12hostmode and key == keyboard.Key.f2:
            send_key(bytes([LOAD, 0x00, 0x80]))
        # send data
        elif f12hostmode and key == keyboard.Key.f4:
            tcflush(sys.stdin, TCIFLUSH)
            filename = input("filename? ")
            print(f"load {filename}")
            tcflush(sys.stdin, TCIFLUSH)
            data_array = read_file_to_byte_array(re.sub("[^A-Z\.]", "", filename, 0, re.IGNORECASE))
            if data_array:
                numofbytes = len(data_array)-2
                if numofbytes < 1:
                    print("data must contain at least 3 bytes")
                    sys.exit(1)
                address = data_array[0] + data_array[1]*256
                dataminidx = 2
                cmdsendbytes = bytes([RECEIVEDATA, 0x00, 0x80])
                # send data in packages of max 248 bytes
                while True:
                    min = numofbytes if numofbytes < 248 else 248
                    # prepare data to send
                    params = min.to_bytes(1, 'little') + address.to_bytes(2, 'little') + data_array[dataminidx:dataminidx+min]
                    if cmdsendbytes is None:
                        prg_array = params
                    else:
                        prg_array = cmdsendbytes + params
                    #formatted_output = ", ".join([f"0x{byte:02X}" for byte in prg_array])
                    #print(f"data to be sent: {formatted_output}")
                    # send data to esp32
                    device.char_write(characteristic_uuid, prg_array)
                    value = device.char_read(characteristic_uuid)
                    cmdsendbytes = None
                    print(f"return value from send data: {value}")
                    if numofbytes <= 248:
                        address += numofbytes
                        break
                    numofbytes -= 248
                    dataminidx += 248
                    address += 248
                # init basic
                prg_array = bytes([0xff])
                device.char_write(characteristic_uuid, prg_array)
                value = device.char_read(characteristic_uuid)
                print(f"return value from initbasic: {value}")
        # show registers
        elif f12hostmode and key == keyboard.Key.f1:
            send_key(bytes([SHOWREG, 0x00, 0x80]))
        # show memory
        elif f12hostmode and key == keyboard.Key.f3:
            tcflush(sys.stdin, TCIFLUSH)
            address = int(input("address (hex)? "), base=16)
            numofbytes = int(input("numofbytes? "))
            if numofbytes > 250:
                numofbytes = 250
            tcflush(sys.stdin, TCIFLUSH)
            device.char_write(characteristic_uuid, bytes([SHOWMEM, 0x00, 0x80]) + numofbytes.to_bytes(1, 'little') + address.to_bytes(2, 'little'))
            value = device.char_read(characteristic_uuid)
        # reset C64
        elif f12hostmode and key == keyboard.Key.f6:
            send_key(bytes([RESET, 0x00, 0x80]))
        # toogle between 'draw each line in row' and 'draw even and odd lines successively'
        elif f12hostmode and key == keyboard.Key.f7:
            send_key(bytes([TOGGLEVICDRAW, 0x00, 0x80]))
        # set keyboard joystick
        elif key == keyboard.Key.f5:
            send_key(bytes([SETKBJOYSTICKMODE, 0x00, 0x80]))
            kbjoystick += 1;
            if kbjoystick > 2:
              kbjoystick = 0;
            print(f"joymode = {kbjoystick}")
        # set joystick
        elif key == keyboard.Key.f9:
            send_key(bytes([SETJOYSTICKMODE, 0x00, 0x80]))
            realjoystick += 1;
            if realjoystick > 2:
              realjoystick = 0;
            print(f"joymode = {realjoystick}")
        elif kbjoystick == 1 and key == keyboard.Key.right:
            send_key(bytes([0xf7, 0x01, 0x20]))
        elif kbjoystick == 1 and key == keyboard.Key.left:
            send_key(bytes([0xfb, 0x01, 0x20]))
        elif kbjoystick == 1 and key == keyboard.Key.down:
            send_key(bytes([0xfd, 0x01, 0x20]))
        elif kbjoystick == 1 and key == keyboard.Key.up:
            send_key(bytes([0xfe, 0x01, 0x20]))
        elif kbjoystick == 1 and key == keyboard.Key.ctrl:
            send_key(bytes([0xef, 0x01, 0x20]))
        elif kbjoystick == 2 and key == keyboard.Key.right:
            send_key(bytes([0x77, 0x01, 0x40]))
        elif kbjoystick == 2 and key == keyboard.Key.left:
            send_key(bytes([0x7b, 0x01, 0x40]))
        elif kbjoystick == 2 and key == keyboard.Key.down:
            send_key(bytes([0x7d, 0x01, 0x40]))
        elif kbjoystick == 2 and key == keyboard.Key.up:
            send_key(bytes([0x7e, 0x01, 0x40]))
        elif kbjoystick == 2 and key == keyboard.Key.ctrl:
            send_key(bytes([0x6f, 0x01, 0x40]))
        # unshifted
        elif key.char == '3':
            send_key(bytes([0xfd, 0xfe, 0x00]))
        elif key.char == 'w':
            send_key(bytes([0xfd, 0xfd, 0x00]))
        elif key.char == 'a':
            send_key(bytes([0xfd, 0xfb, 0x00]))
        elif key.char == '4':
            send_key(bytes([0xfd, 0xf7, 0x00]))
        elif key.char == 'z':
            send_key(bytes([0xfd, 0xef, 0x00]))
        elif key.char == 's':
            send_key(bytes([0xfd, 0xdf, 0x00]))
        elif key.char == 'e':
            send_key(bytes([0xfd, 0xbf, 0x00]))
        elif key.char == '5':
            send_key(bytes([0xfb, 0xfe, 0x00]))
        elif key.char == 'r':
            send_key(bytes([0xfb, 0xfd, 0x00]))
        elif key.char == 'd':
            send_key(bytes([0xfb, 0xfb, 0x00]))
        elif key.char == '6':
            send_key(bytes([0xfb, 0xf7, 0x00]))
        elif key.char == 'c':
            send_key(bytes([0xfb, 0xef, 0x00]))
        elif key.char == 'f':
            send_key(bytes([0xfb, 0xdf, 0x00]))
        elif key.char == 't':
            send_key(bytes([0xfb, 0xbf, 0x00]))
        elif key.char == 'x':
            send_key(bytes([0xfb, 0x7f, 0x00]))
        elif key.char == '7':
            send_key(bytes([0xf7, 0xfe, 0x00]))
        elif key.char == 'y':
            send_key(bytes([0xf7, 0xfd, 0x00]))
        elif key.char == 'g':
            send_key(bytes([0xf7, 0xfb, 0x00]))
        elif key.char == '8':
            send_key(bytes([0xf7, 0xf7, 0x00]))
        elif key.char == 'b':
            send_key(bytes([0xf7, 0xef, 0x00]))
        elif key.char == 'h':
            send_key(bytes([0xf7, 0xdf, 0x00]))
        elif key.char == 'u':
            send_key(bytes([0xf7, 0xbf, 0x00]))
        elif key.char == 'v':
            send_key(bytes([0xf7, 0x7f, 0x00]))
        elif key.char == '9':
            send_key(bytes([0xef, 0xfe, 0x00]))
        elif key.char == 'i':
            send_key(bytes([0xef, 0xfd, 0x00]))
        elif key.char == 'j':
            send_key(bytes([0xef, 0xfb, 0x00]))
        elif key.char == '0':
            send_key(bytes([0xef, 0xf7, 0x00]))
        elif key.char == 'm':
            send_key(bytes([0xef, 0xef, 0x00]))
        elif key.char == 'k':
            send_key(bytes([0xef, 0xdf, 0x00]))
        elif key.char == 'o':
            send_key(bytes([0xef, 0xbf, 0x00]))
        elif key.char == 'n':
            send_key(bytes([0xef, 0x7f, 0x00]))
        elif key.char == '+':
            send_key(bytes([0xdf, 0xfe, 0x00]))
        elif key.char == 'p':
            send_key(bytes([0xdf, 0xfd, 0x00]))
        elif key.char == 'l':
            send_key(bytes([0xdf, 0xfb, 0x00]))
        elif key.char == '-':
            send_key(bytes([0xdf, 0xf7, 0x00]))
        elif key.char == '.':
            send_key(bytes([0xdf, 0xef, 0x00]))
        elif key.char == ':':
            send_key(bytes([0xdf, 0xdf, 0x00]))
        elif key.char == 'ä': # @
            send_key(bytes([0xdf, 0xbf, 0x00]))
        elif key.char == ',':
            send_key(bytes([0xdf, 0x7f, 0x00]))
        elif key.char == '£':
            send_key(bytes([0xbf, 0xfe, 0x00]))
        elif key.char == '*':
            send_key(bytes([0xbf, 0xfd, 0x00]))
        elif key.char == ';':
            send_key(bytes([0xbf, 0xfb, 0x00]))
        elif key.char == '=':
            send_key(bytes([0xbf, 0xdf, 0x00]))
        elif key.char == '^':
            send_key(bytes([0xbf, 0xbf, 0x00]))
        elif key.char == '/':
            send_key(bytes([0xbf, 0x7f, 0x00]))
        elif key.char == '1':
            send_key(bytes([0x7f, 0xfe, 0x00]))
        elif key.char == '§':
            send_key(bytes([0x7f, 0xfd, 0x00]))
        elif key.char == '2':
            send_key(bytes([0x7f, 0xf7, 0x00]))
        elif key.char == 'q':
            send_key(bytes([0x7f, 0xbf, 0x00]))
        # shifted
        elif key.char == '!':
            send_key(bytes([0x7f, 0xfe, 0x01]))
        elif key.char == '"':
            send_key(bytes([0x7f, 0xf7, 0x01]))
        elif key.char == 'ö': # '#'
            send_key(bytes([0xfd, 0xfe, 0x01]))
        elif key.char == '$':
            send_key(bytes([0xfd, 0xf7, 0x01]))
        elif key.char == '%':
            send_key(bytes([0xfb, 0xfe, 0x01]))
        elif key.char == '&':
            send_key(bytes([0xfb, 0xf7, 0x01]))
        elif key.char == '\'':
            send_key(bytes([0xf7, 0xfe, 0x01]))
        elif key.char == '(':
            send_key(bytes([0xf7, 0xf7, 0x01]))
        elif key.char == ')':
            send_key(bytes([0xef, 0xfe, 0x01]))
        elif key.char == 'ü': # '['
            send_key(bytes([0xdf, 0xdf, 0x01]))
        elif key.char == '¨': # ']'
            send_key(bytes([0xbf, 0xfb, 0x01]))
        elif key.char == '<':
            send_key(bytes([0xdf, 0x7f, 0x01]))
        elif key.char == '>':
            send_key(bytes([0xdf, 0xef, 0x01]))
        elif key.char == '?':
            send_key(bytes([0xbf, 0x7f, 0x01]))
    except AttributeError:
        return None

#def on_release(key):
#    if kbjoystick == 1:
#        send_key(bytes([0xff, 0x00, 0x20]))
#    elif kbjoystick == 2:
#        send_key(bytes([0x7f, 0x00, 0x20]))


if __name__ == "__main__":
    # get active window
    activewin = get_active_window()

    # print some info
    print("f12: toggle between external and c64 mode for f1-f8 (default: c64 mode)")
    print("f9: set joystick")
    print("f1: show cpu + chip registers")
    print("f2: load prg from sdcard")
    print("f3: show memory")
    print("f4: send prg to esp by BLE")
    print("f5: set keyboard joystick")
    print("f6: reset C64")
    print("f7: toggle between 'draw each line one after another' and 'draw even and odd lines successively'")

    # connect to BLE server
    adapter = pygatt.GATTToolBackend()
    adapter.start()
    device = adapter.connect(device_address)
    print("connected")

    # listen to keyboard
    # with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    with keyboard.Listener(on_press=on_press) as listener:
        listener.join()

