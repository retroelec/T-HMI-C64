package org.retroelec.thmic64kb;

import android.app.Application;

public class MyApplication extends Application {
    private BLEManager bleManager;
    private Settings settings;
    private Type4Notification type4Notification;
    private Type5Notification type5Notification;
    private Type6Notification type6Notification;
    private boolean bleTransferActive;

    public BLEManager getBleManager() {
        return bleManager;
    }

    public void setBleManager(BLEManager bleManager) {
        this.bleManager = bleManager;
    }

    public Settings getSettings() {
        return settings;
    }

    public void setSettings(Settings settings) {
        this.settings = settings;
    }

    public Type4Notification getType4Notification() {
        return type4Notification;
    }

    public void setType4Notification(Type4Notification type4Notification) {
        this.type4Notification = type4Notification;
    }

    public Type5Notification getType5Notification() {
        return type5Notification;
    }

    public void setType5Notification(Type5Notification type5Notification) {
        this.type5Notification = type5Notification;
    }

    public Type6Notification getType6Notification() {
        return type6Notification;
    }

    public void setType6Notification(Type6Notification type6Notification) {
        this.type6Notification = type6Notification;
    }

    public boolean isBleTransferActive() {
        return bleTransferActive;
    }

    public void setBleTransferActive(boolean bleTransferActive) {
        this.bleTransferActive = bleTransferActive;
    }
}