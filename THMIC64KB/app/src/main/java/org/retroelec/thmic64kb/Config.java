package org.retroelec.thmic64kb;

import java.util.UUID;

public class Config {
    public static final UUID THMIC64_CHARACTERISTIC_UUID = UUID.fromString("3b05e9bf-086f-4b56-9c37-7b7eeb30b28b");
    public static final UUID THMIC64_SERVICE_UUID = UUID.fromString("695ba701-a48c-43f6-9028-3c885771f19f");
    public static final String TARGET_DEVICE_NAME = "THMIC64";
    public static final long CHECK_INTERVAL = 1000;
    public static final long BATTERY_CHECK_INTERVAL = 5 * 60 * 1000; // 5 minutes
    public static final long BATTERY_CHECK_FORFIRSTTIME = 10 * 1000; // 10 seconds
    public static final long SEARCH_DURATION = 10000;
    public static final short DEFAULT_MINKEYPRESSEDDURATION = 130;
    public static final int DURATION_VIBRATION_EFFECT = 100;
    public static final int DURATION_VISUAL_EFFECT = 200;

    public static final int KEYSELECTEDCOLORACTION = 0xffdd7777;
    public static final int KEYBGDCOLORACTION = 0xffcc7777;
    public static final int KEYBGDCOLORD64ATTACHED = 0xff007777;

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

    public static final byte KEYRELEASED = (byte) 0xff;

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
    public static final byte SENDRAWKEYS = 24;
    public static final byte SWITCHDEBUG = 25;
    public static final byte SWITCHPERF = 26;
    public static final byte SWITCHDETECTRELEASEKEY = 27;
    public static final byte GETBATTERYVOLTAGE = 29;
    public static final byte POWEROFF = 30;
    public static final byte SAVE = 31;
    public static final byte LIST = 32;
    public static final byte SETVOLUME = 33;
    public static final byte ATTACHD64 = 37;
    public static final byte DETACHD64 = 38;
}
