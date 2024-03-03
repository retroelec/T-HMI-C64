package org.retroelec.thmic64kb;

import android.app.Application;
import android.bluetooth.BluetoothGattCharacteristic;

public class Globals extends Application {
    private BLEManager bleManager;
    private BluetoothGattCharacteristic characteristic;

    public BLEManager getBleManager() {
        return bleManager;
    }

    public void setBleManager(BLEManager bleManager) {
        this.bleManager = bleManager;
    }

    public BluetoothGattCharacteristic getCharacteristic() {
        return characteristic;
    }

    public void setCharacteristic(BluetoothGattCharacteristic characteristic) {
        this.characteristic = characteristic;
    }
}
