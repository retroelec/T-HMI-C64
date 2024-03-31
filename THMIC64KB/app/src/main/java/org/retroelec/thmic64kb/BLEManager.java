package org.retroelec.thmic64kb;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.Context;
import android.util.Log;

import java.util.UUID;

public class BLEManager {
    private static final UUID THMIC64_CHARACTERISTIC_UUID = UUID.fromString("3b05e9bf-086f-4b56-9c37-7b7eeb30b28b");
    private static final UUID THMIC64_SERVICE_UUID = UUID.fromString("695ba701-a48c-43f6-9028-3c885771f19f");
    private BluetoothGatt bluetoothGatt;
    private BluetoothGattCharacteristic characteristic = null;
    private final Context context;

    public BLEManager(Context context) {
        this.context = context;
    }

    public BluetoothGattCharacteristic getCharacteristic() {
        return characteristic;
    }

    public void connectToDevice(Context context, BluetoothDevice device) {
        context.getSystemService(Context.BLUETOOTH_SERVICE);
        bluetoothGatt = device.connectGatt(context, false, gattCallback);
    }

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);
            if (newState == BluetoothGatt.STATE_CONNECTED) {
                gatt.discoverServices();
            } else if (newState == BluetoothGatt.STATE_DISCONNECTED) {
                characteristic = null;
                final Globals globals = (Globals) context.getApplicationContext();
                globals.setCharacteristic(characteristic);
                Log.d("THMIC64", "BluetoothGatt.STATE_DISCONNECTED");
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                BluetoothGattService service = gatt.getService(THMIC64_SERVICE_UUID);
                if (service != null) {
                    characteristic = service.getCharacteristic(THMIC64_CHARACTERISTIC_UUID);
                    if (characteristic != null) {
                        Log.d("THMIC64", "Characteristic found successfully.");
                        final Globals globals = (Globals) context.getApplicationContext();
                        globals.setCharacteristic(characteristic);
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

        /*
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
            if (characteristic.getUuid().equals(THMIC64_CHARACTERISTIC_UUID)) {
                byte[] receivedData = characteristic.getValue();
                if (receivedData.length == 1) {
                    byte responseByte = receivedData[0];
                    if (responseByte != 'K') {
                        Log.e("THMIC64", "ble answer: wrong byte value");
                    }
                } else {
                    Log.e("THMIC64", "ble answer: wrong number of bytes");
                }
            } else {
                Log.e("THMIC64", "ble answer: wrong characteristic");
            }
        }
        */
    };

    public void sendData(byte[] data) {
        characteristic.setValue(data);
        bluetoothGatt.writeCharacteristic(characteristic);
    }

    public void disconnect() {
        characteristic = null;
        final Globals globals = (Globals) context.getApplicationContext();
        globals.setCharacteristic(characteristic);
        if (bluetoothGatt != null) {
            bluetoothGatt.disconnect();
            bluetoothGatt.close();
        }
    }
}
