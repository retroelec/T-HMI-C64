package org.retroelec.thmic64kb;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Button;
import android.widget.Switch;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.util.Objects;

public class MainActivity extends AppCompatActivity {

    public static final String TARGET_DEVICE_NAME = "THMIC64";
    private static final int PERMISSION_REQUEST_CODE = 1234;
    private static final long CHECK_INTERVAL = 1000;
    private static final long SEARCH_DURATION = 10000;
    private BLEManager bleManager = null;
    private BluetoothLeScanner bluetoothLeScanner;
    private Switch bleSwitch;
    private Button keydiv;
    private Button keykbjoystick1;
    private Button keykbjoystick2;
    private Button joystick1;
    private Button joystick2;
    private boolean joystick1active = false;
    private boolean joystick2active = false;
    private final Handler handler = new Handler();

    private void connectToDevice() {
        ScanCallback scanCallback = new ScanCallback() {
            @Override
            public void onScanResult(int callbackType, @NonNull ScanResult result) {
                super.onScanResult(callbackType, result);
                BluetoothDevice device = result.getDevice();
                if (device != null && TARGET_DEVICE_NAME.equals(device.getName())) {
                    bleManager.connectToDevice(MainActivity.this, device);
                }
            }

            @Override
            public void onScanFailed(int errorCode) {
                super.onScanFailed(errorCode);
                Toast.makeText(MainActivity.this, "Could not find " + TARGET_DEVICE_NAME + " device", Toast.LENGTH_SHORT).show();
            }
        };

        // start scan
        bluetoothLeScanner.startScan(scanCallback);

        // search for SEARCH_DURATION milliseconds
        new Handler(Looper.getMainLooper()).postDelayed(() -> bluetoothLeScanner.stopScan(scanCallback), SEARCH_DURATION);
    }

    // check BLE connection
    private void startPeriodicCheck() {
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                final Globals globals = (Globals) getApplicationContext();
                BluetoothGattCharacteristic characteristic = globals.getCharacteristic();
                bleSwitch.setChecked(characteristic != null);
                handler.postDelayed(this, CHECK_INTERVAL);
            }
        }, CHECK_INTERVAL);
    }

    private void main() {
        // init ble
        bleManager = new BLEManager(this);
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner();
        if (!bluetoothAdapter.isEnabled() || bluetoothLeScanner == null) {
            Toast.makeText(this, "bluetooth not available, please close app", Toast.LENGTH_LONG).show();
            return;
        }

        // search for device + connect to device
        connectToDevice();
        final Globals globals = (Globals) getApplicationContext();
        globals.setBleManager(bleManager);

        // add C64 virtual keyboard
        findViewById(R.id.keyboard);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (bleManager != null) {
            bleManager.disconnect();
        }
        handler.removeCallbacksAndMessages(null);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSION_REQUEST_CODE) {
            if ((ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED)
                    && (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED)
                    && (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED)) {
                main();
            } else {
                Toast.makeText(this, "permissions not granted, please close app", Toast.LENGTH_SHORT).show();
            }
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Objects.requireNonNull(getSupportActionBar()).hide();
        setContentView(R.layout.activity_main);

        keydiv = findViewById(R.id.keydiv);
        keydiv.setOnClickListener(view -> {
            Intent i = new Intent(MainActivity.this, DivActivity.class);
            startActivity(i);
        });

        keykbjoystick1 = findViewById(R.id.keykbjoystick1);
        keykbjoystick1.setOnClickListener(view -> {
            if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
                bleManager.sendData(new byte[]{(byte) 3, (byte) 0x00, (byte) 0x80});
            }
            Intent i = new Intent(MainActivity.this, KBJoystickActivity.class);
            startActivity(i);
        });

        keykbjoystick2 = findViewById(R.id.keykbjoystick2);
        keykbjoystick2.setOnClickListener(view -> {
            if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
                bleManager.sendData(new byte[]{(byte) 4, (byte) 0x00, (byte) 0x80});
            }
            Intent i = new Intent(MainActivity.this, KBJoystickActivity.class);
            startActivity(i);
        });

        joystick1 = findViewById(R.id.joystick1);
        joystick1.setOnClickListener(view -> {
            if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
                if (!joystick1active) {
                    // activate joystick 1
                    joystick1.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#77cc77")));
                    bleManager.sendData(new byte[]{(byte) 1, (byte) 0x00, (byte) 0x80});
                    // deactivate joystick 2
                    joystick2active = false;
                    joystick2.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#cc7777")));
                } else {
                    // deactivate joystick 1
                    joystick1.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#cc7777")));
                    bleManager.sendData(new byte[]{(byte) 5, (byte) 0x00, (byte) 0x80});
                }
                joystick1active = !joystick1active;
            }
        });

        joystick2 = findViewById(R.id.joystick2);
        joystick2.setOnClickListener(view -> {
            if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
                if (!joystick2active) {
                    // activate joystick 2
                    joystick2.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#77cc77")));
                    bleManager.sendData(new byte[]{(byte) 2, (byte) 0x00, (byte) 0x80});
                    // deactivate joysticks
                    joystick1active = false;
                    joystick1.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#cc7777")));
                } else {
                    // deactivate joystick 2
                    joystick2.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#cc7777")));
                    bleManager.sendData(new byte[]{(byte) 5, (byte) 0x00, (byte) 0x80});
                }
                joystick2active = !joystick2active;
            }
        });

        bleSwitch = findViewById(R.id.bleSwitch);
        bleSwitch.setChecked(false);
        bleSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) {
                // activate BLE
                connectToDevice();
            }
        });
        startPeriodicCheck();

        if ((ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED)
                && (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED)
                && (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED)) {
            main();
        } else {
            Log.d("THMIC64", "ask for permissions");
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.BLUETOOTH_CONNECT, Manifest.permission.BLUETOOTH_SCAN,
                    Manifest.permission.ACCESS_FINE_LOCATION}, PERMISSION_REQUEST_CODE);
        }
    }
}