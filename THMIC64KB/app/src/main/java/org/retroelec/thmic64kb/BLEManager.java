package org.retroelec.thmic64kb;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.NonNull;

public class BLEManager {
    private BluetoothGatt bluetoothGatt = null;
    private BluetoothGattCharacteristic characteristic = null;
    private final BluetoothLeScanner bluetoothLeScanner;
    private final MainActivity mainActivity;
    private final String targetDeviceName;
    private final Settings settings;
    private final Type2Notification type2Notification;
    private final Type3Notification type3Notification;
    private final Type4Notification type4Notification;
    private final Type5Notification type5Notification;

    private boolean servicesDiscovered = false;
    private boolean isWriteInProgress = false;

    public BluetoothGattCharacteristic getCharacteristic() {
        return characteristic;
    }

    public BLEManager(MainActivity mainActivity, String targetDeviceName, Settings settings, Type2Notification type2Notification, Type3Notification type3Notification, Type4Notification type4Notification, Type5Notification type5Notification) {
        this.mainActivity = mainActivity;
        this.targetDeviceName = targetDeviceName;
        this.settings = settings;
        this.type2Notification = type2Notification;
        this.type3Notification = type3Notification;
        this.type4Notification = type4Notification;
        this.type5Notification = type5Notification;
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner();
        if (!bluetoothAdapter.isEnabled() || bluetoothLeScanner == null) {
            Toast.makeText(mainActivity, "bluetooth not available, please close app", Toast.LENGTH_LONG).show();
        }
    }

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);
            if (newState == BluetoothGatt.STATE_CONNECTED) {
                if (!servicesDiscovered) {
                    Log.i("THMIC64", "connected to GATT server");
                    gatt.discoverServices();
                    servicesDiscovered = true;
                } else {
                    return;
                }
            } else if (newState == BluetoothGatt.STATE_DISCONNECTED) {
                Log.i("THMIC64", "disconnected from GATT server");
                servicesDiscovered = false;
                disconnect();
            }
            characteristic = null;
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            if ((status == BluetoothGatt.GATT_SUCCESS) && (characteristic == null)) {
                BluetoothGattService service = gatt.getService(Config.THMIC64_SERVICE_UUID);
                if (service != null) {
                    characteristic = service.getCharacteristic(Config.THMIC64_CHARACTERISTIC_UUID);
                    if (characteristic != null) {
                        Log.i("THMIC64", "Characteristic found successfully.");
                        gatt.setCharacteristicNotification(characteristic, true);
                    } else {
                        Log.e("THMIC64", "Characteristic not found.");
                    }
                } else {
                    Log.e("THMIC64", "Service not found.");
                }
            } else {
                Log.e("THMIC64", "onServicesDiscovered error: " + status);
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.w("BLEManager", "Characteristic write failed with status: " + status);
            }
            isWriteInProgress = false;
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            Log.d("THMIC64", "characteristic changed");
            super.onCharacteristicChanged(gatt, characteristic);
            if (characteristic.getUuid().equals(Config.THMIC64_CHARACTERISTIC_UUID)) {
                byte[] receivedData = characteristic.getValue();
                Log.i("THMIC64", "received notification, len = " + receivedData.length);
                if (receivedData.length > 0) {
                    byte type = receivedData[0];
                    switch (type) {
                        // state of joysticks and switches
                        case 1:
                            settings.setJoymode(receivedData[1]);
                            settings.setDeactivateTemp(receivedData[2] != 0);
                            settings.setSendRawKeyCodes(receivedData[3] != 0);
                            settings.setDebug(receivedData[4] != 0);
                            settings.setPerf(receivedData[5] != 0);
                            settings.setDetectReleaseKey(receivedData[6] != 0);
                            settings.notifySettingsObserver();
                            break;
                        case 2:
                            type2Notification.setCpuRunning(receivedData[1] != 0);
                            int pcl = receivedData[2] & 0xff;
                            int pch = receivedData[3] & 0xff;
                            type2Notification.setPc(pcl | (pch << 8));
                            type2Notification.setA(receivedData[4]);
                            type2Notification.setX(receivedData[5]);
                            type2Notification.setY(receivedData[6]);
                            type2Notification.setSr(receivedData[7]);
                            type2Notification.setD011(receivedData[8]);
                            type2Notification.setD016(receivedData[9]);
                            type2Notification.setD018(receivedData[10]);
                            type2Notification.setD019(receivedData[11]);
                            type2Notification.setD01a(receivedData[12]);
                            type2Notification.setRegister1(receivedData[13]);
                            type2Notification.setDc0d(receivedData[14]);
                            type2Notification.setDc0e(receivedData[15]);
                            type2Notification.setDc0f(receivedData[16]);
                            type2Notification.setDd0d(receivedData[17]);
                            type2Notification.setDd0e(receivedData[18]);
                            type2Notification.setDd0f(receivedData[19]);
                            type2Notification.notifyObserver();
                            break;
                        case 3:
                            short[] mem = new short[16];
                            for (byte i = 0; i < 16; i++) {
                                mem[i] = receivedData[i + 1];
                            }
                            type3Notification.setMem(mem);
                            type3Notification.notifyObserver();
                            break;
                        // used to send data from android to esp
                        case 4:
                            type4Notification.notifyObserver();
                            break;
                        // battery voltage
                        case 5:
                            int batvoll = receivedData[1] & 0xff;
                            int batvolh = receivedData[2] & 0xff;
                            Log.i("THMIC64", "battery voltage = " + Integer.toString(batvoll | (batvolh << 8)));
                            type5Notification.setBatteryVoltage(batvoll | (batvolh << 8));
                            mainActivity.runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    type5Notification.notifyObserver();
                                }
                            });
                            break;
                    }
                } else {
                    Log.e("THMIC64", "received notification: wrong length");
                }
            } else {
                Log.e("THMIC64", "received notification: wrong characteristic");
            }
        }
    };

    private void connectToDevice(BluetoothDevice device) {
        mainActivity.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothGatt == null) {
            bluetoothGatt = device.connectGatt(mainActivity, false, gattCallback);
        } else {
            Log.i("THMIC64", "already connected to GATT server");
        }
    }

    public void scanForDevice() {
        if (bluetoothLeScanner == null) {
            return;
        }
        ScanCallback scanCallback = new ScanCallback() {
            @Override
            public void onScanResult(int callbackType, @NonNull ScanResult result) {
                super.onScanResult(callbackType, result);
                BluetoothDevice device = result.getDevice();
                if (device != null && targetDeviceName.equals(device.getName())) {
                    Log.d("THMIC64", "connect to device");
                    connectToDevice(device);
                }
            }

            @Override
            public void onScanFailed(int errorCode) {
                super.onScanFailed(errorCode);
                Toast.makeText(mainActivity, "Could not find " + targetDeviceName + " device", Toast.LENGTH_SHORT).show();
            }
        };

        // start scan
        bluetoothLeScanner.startScan(scanCallback);

        // search for SEARCH_DURATION milliseconds
        new Handler(Looper.getMainLooper()).postDelayed(() -> bluetoothLeScanner.stopScan(scanCallback), Config.SEARCH_DURATION);
    }

    public void sendData(byte[] data, boolean blocking) {
        if (blocking) {
            while (isWriteInProgress) {
            }
            isWriteInProgress = true;
        }
        characteristic.setValue(data);
        bluetoothGatt.writeCharacteristic(characteristic);
    }

    public void disconnect() {
        characteristic = null;
        if (bluetoothGatt != null) {
            bluetoothGatt.disconnect();
            bluetoothGatt.close();
            bluetoothGatt = null;
        }
    }
}
