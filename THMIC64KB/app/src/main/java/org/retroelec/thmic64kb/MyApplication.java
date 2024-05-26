package org.retroelec.thmic64kb;

import android.app.Application;

public class MyApplication extends Application {
    private BLEManager bleManager;
    private Settings settings;
    private Type2Notification type2Notification;

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

    public Type2Notification getType2Notification() {
        return type2Notification;
    }

    public void setType2Notification(Type2Notification type2Notification) {
        this.type2Notification = type2Notification;
    }
}
