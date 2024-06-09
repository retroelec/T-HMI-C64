package org.retroelec.thmic64kb;

import java.util.UUID;

public class Config {
    public static final UUID THMIC64_CHARACTERISTIC_UUID = UUID.fromString("3b05e9bf-086f-4b56-9c37-7b7eeb30b28b");
    public static final UUID THMIC64_SERVICE_UUID = UUID.fromString("695ba701-a48c-43f6-9028-3c885771f19f");
    public static final String TARGET_DEVICE_NAME = "THMIC64";
    public static final long CHECK_INTERVAL = 1000;
    public static final long SEARCH_DURATION = 10000;

    public static final byte VIRTUALJOYSTICKLEFT_ACTIVATED = (byte) 0x02;
    public static final byte VIRTUALJOYSTICKLEFT_DEACTIVATED = (byte) 0x82;
    public static final byte VIRTUALJOYSTICKRIGHT_ACTIVATED = (byte) 0x03;
    public static final byte VIRTUALJOYSTICKRIGHT_DEACTIVATED = (byte) 0x83;
    public static final byte VIRTUALJOYSTICKUP_ACTIVATED = (byte) 0x00;
    public static final byte VIRTUALJOYSTICKUP_DEACTIVATED = (byte) 0x80;
    public static final byte VIRTUALJOYSTICKDOWN_ACTIVATED = (byte) 0x01;
    public static final byte VIRTUALJOYSTICKDOWN_DEACTIVATED = (byte) 0x81;
    public static final byte VIRTUALJOYSTICKFIRE_ACTIVATED = (byte) 0x04;
    public static final byte VIRTUALJOYSTICKFIRE_DEACTIVATED = (byte) 0x84;

    public static final byte JOYSTICKMODE1 = 1;
    public static final byte JOYSTICKMODE2 = 2;
    public static final byte KBJOYSTICKMODE1 = 3;
    public static final byte KBJOYSTICKMODE2 = 4;
    public static final byte JOYSTICKMODEOFF = 5;
    public static final byte KBJOYSTICKMODEOFF = 6;
    public static final byte LOAD = 11;
    public static final byte RECEIVEDATA = 12;
    public static final byte SHOWREG = 13;
    public static final byte SHOWMEM = 14;
    public static final byte RESTORE = 15;
    public static final byte RESET = 20;
    public static final byte GETSTATUS = 21;
    public static final byte SWITCHFRAMECOLORREFRESH = 22;
    public static final byte SWITCHCIA2 = 23;
    public static final byte SENDRAWKEYS = 24;
}
