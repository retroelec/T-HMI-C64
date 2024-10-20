package org.retroelec.thmic64kb;

import android.Manifest;
import android.bluetooth.BluetoothGattCharacteristic;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.Switch;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class MainActivity extends AppCompatActivity implements SettingsObserver {

    private static final int PERMISSION_REQUEST_CODE = 1234;
    private BLEManager bleManager = null;
    private Settings settings;
    private Type2Notification type2Notification;
    private Type3Notification type3Notification;
    private Type4Notification type4Notification;
    private Switch bleSwitch;
    private Button joystick1;
    private Button joystick2;
    private boolean joystick1active = false;
    private boolean joystick2active = false;
    private ImageButton powerOff;
    private final Handler handler = new Handler();
    private BluetoothGattCharacteristic oldcharacteristic;
    private BluetoothGattCharacteristic actcharacteristic = null;

    // check BLE connection
    private void startPeriodicCheck() {
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (bleManager != null) {
                    oldcharacteristic = actcharacteristic;
                    actcharacteristic = bleManager.getCharacteristic();
                    bleSwitch.setChecked(actcharacteristic != null);
                    if ((oldcharacteristic == null) && (actcharacteristic != null)) {
                        // get initial settings
                        bleManager.sendData(new byte[]{Config.GETSTATUS, (byte) 0x00, (byte) 0x80});
                    }
                    handler.postDelayed(this, Config.CHECK_INTERVAL);
                } else {
                    actcharacteristic = null;
                }
            }
        }, Config.CHECK_INTERVAL);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (bleManager != null) {
            bleManager.disconnect();
        }
        handler.removeCallbacksAndMessages(null);
    }

    private void scanForBLEDevice() {
        // init BLEManager
        Log.d("THMIC64", "init BLEManager");
        bleManager = new BLEManager(this, Config.TARGET_DEVICE_NAME, settings, type2Notification, type3Notification, type4Notification);
        final MyApplication myApplication = (MyApplication) getApplication();
        myApplication.setBleManager(bleManager);

        if (bleManager != null) {
            // search for device + connect to device
            Log.d("THMIC64", "scan for device");
            bleManager.scanForDevice();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSION_REQUEST_CODE) {
            if ((ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED)
                    && (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED)
                    && (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED)) {
                scanForBLEDevice();
            } else {
                Toast.makeText(this, "permissions not granted, please close app", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void refreshActiveJoystickButtons(boolean joystick1active, boolean joystick2active) {
        if (joystick1active) {
            joystick1.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#77cc77")));
            joystick2.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#cc7777")));
        } else if (joystick2active) {
            joystick2.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#77cc77")));
            joystick1.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#cc7777")));
        } else {
            joystick1.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#cc7777")));
            joystick2.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#cc7777")));
        }
    }

    @Override
    public void updateSettings() {
        byte joymode = settings.getJoymode();
        boolean joy1active = (joymode == 1);
        boolean joy2active = (joymode == 2);
        refreshActiveJoystickButtons(joy1active, joy2active);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (settings != null) {
            settings.registerSettingsObserver(this);
        }
        updateSettings();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (settings != null) {
            settings.removeSettingsObserver();
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // init notifications
        settings = new Settings();
        settings.registerSettingsObserver(this);
        settings.setMinKeyPressedDuration(Config.DEFAULT_MINKEYPRESSEDDURATION);
        type2Notification = new Type2Notification();
        type3Notification = new Type3Notification();
        type4Notification = new Type4Notification();
        final MyApplication myApplication = (MyApplication) getApplication();
        myApplication.setSettings(settings);
        myApplication.setType2Notification(type2Notification);
        myApplication.setType3Notification(type3Notification);
        myApplication.setType4Notification(type4Notification);

        setContentView(R.layout.activity_main);

        Button keydiv = findViewById(R.id.keydiv);
        keydiv.setOnClickListener(view -> {
            Intent i = new Intent(MainActivity.this, DivActivity.class);
            startActivity(i);
        });

        Button keykbjoystick1 = findViewById(R.id.keykbjoystick1);
        keykbjoystick1.setOnClickListener(view -> {
            if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
                bleManager.sendData(new byte[]{Config.KBJOYSTICKMODE1, (byte) 0x00, (byte) 0x80});
            }
            Intent i = new Intent(MainActivity.this, KBJoystickActivity.class);
            startActivity(i);
        });

        Button keykbjoystick2 = findViewById(R.id.keykbjoystick2);
        keykbjoystick2.setOnClickListener(view -> {
            if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
                bleManager.sendData(new byte[]{Config.KBJOYSTICKMODE2, (byte) 0x00, (byte) 0x80});
            }
            Intent i = new Intent(MainActivity.this, KBJoystickActivity.class);
            i.putExtra("SHOW_FIRE2_BUTTON", true);
            startActivity(i);
        });

        joystick1 = findViewById(R.id.joystick1);
        joystick1.setOnClickListener(view -> {
            if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
                if (!joystick1active) {
                    // activate joystick 1
                    bleManager.sendData(new byte[]{Config.JOYSTICKMODE1, (byte) 0x00, (byte) 0x80});
                    // deactivate joystick 2
                    joystick2active = false;
                } else {
                    // deactivate joystick 1
                    bleManager.sendData(new byte[]{Config.JOYSTICKMODEOFF, (byte) 0x00, (byte) 0x80});
                }
                joystick1active = !joystick1active;
                refreshActiveJoystickButtons(joystick1active, joystick2active);
            }
        });

        joystick2 = findViewById(R.id.joystick2);
        joystick2.setOnClickListener(view -> {
            if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
                if (!joystick2active) {
                    // activate joystick 2
                    bleManager.sendData(new byte[]{Config.JOYSTICKMODE2, (byte) 0x00, (byte) 0x80});
                    // deactivate joysticks
                    joystick1active = false;
                } else {
                    // deactivate joystick 2
                    bleManager.sendData(new byte[]{Config.JOYSTICKMODEOFF, (byte) 0x00, (byte) 0x80});
                }
                joystick2active = !joystick2active;
                refreshActiveJoystickButtons(joystick1active, joystick2active);
            }
        });

        powerOff = findViewById(R.id.powerOff);
        powerOff.setOnClickListener(view -> {
            if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
                bleManager.sendData(new byte[]{Config.POWEROFF, (byte) 0x00, (byte) 0x80});
            }
        });

        bleSwitch = findViewById(R.id.bleSwitch);
        bleSwitch.setChecked(false);
        bleSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) {
                if (bleManager != null) {
                    // activate BLE
                    bleManager.scanForDevice();
                }
            }
        });
        startPeriodicCheck();

        // add C64 virtual keyboard
        findViewById(R.id.keyboard);

        if ((ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED)
                && (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED)
                && (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED)) {
            scanForBLEDevice();
        } else {
            Log.d("THMIC64", "ask for permissions");
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.BLUETOOTH_CONNECT, Manifest.permission.BLUETOOTH_SCAN,
                    Manifest.permission.ACCESS_FINE_LOCATION}, PERMISSION_REQUEST_CODE);
        }
    }
}