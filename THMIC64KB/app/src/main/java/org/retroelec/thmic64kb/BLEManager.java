package org.retroelec.thmic64kb;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
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
    private BluetoothDevice device = null;
    private final BluetoothLeScanner bluetoothLeScanner;
    private final MainActivity mainActivity;
    private final String targetDeviceName;
    private final Settings settings;
    private final Type4Notification type4Notification;
    private final Type5Notification type5Notification;
    private final Type6Notification type6Notification;

    private boolean servicesDiscovered = false;
    private boolean isWriteInProgress = false;
    private ConnectionStateListener connectionStateListener = null;

    public BluetoothGattCharacteristic getCharacteristic() {
        return characteristic;
    }

    public void setConnectionStateListener(ConnectionStateListener connectionStateListener) {
        this.connectionStateListener = connectionStateListener;
    }

    public BLEManager(MainActivity mainActivity, String targetDeviceName, Settings settings, Type4Notification type4Notification, Type5Notification type5Notification, Type6Notification type6Notification) {
        this.mainActivity = mainActivity;
        this.targetDeviceName = targetDeviceName;
        this.settings = settings;
        this.type4Notification = type4Notification;
        this.type5Notification = type5Notification;
        this.type6Notification = type6Notification;
        BluetoothManager bluetoothManager = (BluetoothManager) mainActivity.getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter bluetoothAdapter = (bluetoothManager != null) ? bluetoothManager.getAdapter() : null;
        bluetoothLeScanner = (bluetoothAdapter != null) ? bluetoothAdapter.getBluetoothLeScanner() : null;
        if ((bluetoothAdapter == null) || !bluetoothAdapter.isEnabled() || bluetoothLeScanner == null) {
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
                characteristic = null;
                if (connectionStateListener != null) {
                    connectionStateListener.onDisconnected();
                }
            } else {
                characteristic = null;
            }
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
                        try {
                            BluetoothGattDescriptor descriptor = characteristic.getDescriptor(Config.THMIC64_CCCD_UUID);
                            if (descriptor != null) {
                                gatt.writeDescriptor(descriptor, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                            } else {
                                Log.w("BLEManager", "CCCD descriptor not found, notifying connected");
                                if (connectionStateListener != null) {
                                    connectionStateListener.onConnected();
                                }
                            }
                        } catch (Exception e) {
                            Log.e("BLEManager", "CCCD descriptor write failed", e);
                            if (connectionStateListener != null) {
                                connectionStateListener.onConnected();
                            }
                        }
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
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.i("THMIC64", "CCCD descriptor write success");
            } else {
                Log.w("BLEManager", "Descriptor write failed with status: " + status);
            }
            if (connectionStateListener != null) {
                connectionStateListener.onConnected();
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, byte[] value) {
            Log.d("THMIC64", "characteristic changed");
            super.onCharacteristicChanged(gatt, characteristic, value);
            if (characteristic.getUuid().equals(Config.THMIC64_CHARACTERISTIC_UUID)) {
                byte[] receivedData = value;
                Log.i("THMIC64", "received notification, len = " + receivedData.length);
                if (receivedData.length > 0) {
                    byte type = receivedData[0];
                    switch (type) {
                        // get status (e.g. state of switches, joystick mode, volume)
                        case 1:
                            settings.setJoymode(receivedData[1]);
                            settings.setSendRawKeyCodes(receivedData[3] != 0);
                            settings.setDebug(receivedData[4] != 0);
                            settings.setPerf(receivedData[5] != 0);
                            settings.setDetectReleaseKey(receivedData[6] != 0);
                            settings.setVolume(Byte.toUnsignedInt(receivedData[7]));
                            settings.setD64attached(receivedData[8] != 0);
                            mainActivity.runOnUiThread(settings::notifySettingsObserver);
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
                        // THMIC64KB version
                        case 6:
                            type6Notification.setVersion(receivedData[1] & 0xff);
                            Log.i("THMIC64", "THMIC64KB version: " + (receivedData[1] & 0xff));
                            type6Notification.notifyObserver();
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
            this.device = device;
            bluetoothGatt = device.connectGatt(mainActivity, false, gattCallback);
        } else {
            Log.i("THMIC64", "already connected to GATT server");
        }
    }

    public void reconnect() {
        if (device != null) {
            if (bluetoothGatt != null) {
                bluetoothGatt.disconnect();
                bluetoothGatt.close();
                bluetoothGatt = null;
            }
            bluetoothGatt = device.connectGatt(mainActivity, true, gattCallback);
        }
    }

    public boolean hasDevice() {
        return device != null;
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
        bluetoothGatt.writeCharacteristic(characteristic, data, characteristic.getWriteType());
    }

    public void disconnect() {
        characteristic = null;
        device = null;
        if (bluetoothGatt != null) {
            bluetoothGatt.disconnect();
            bluetoothGatt.close();
            bluetoothGatt = null;
        }
    }
}
