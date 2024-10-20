package org.retroelec.thmic64kb;

import android.app.Application;

public class MyApplication extends Application {
    private BLEManager bleManager;
    private Settings settings;
    private Type2Notification type2Notification;
    private Type3Notification type3Notification;
    private Type4Notification type4Notification;

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

    public Type3Notification getType3Notification() {
        return type3Notification;
    }

    public void setType3Notification(Type3Notification type3Notification) {
        this.type3Notification = type3Notification;
    }

    public Type4Notification getType4Notification() {
        return type4Notification;
    }

    public void setType4Notification(Type4Notification type4Notification) {
        this.type4Notification = type4Notification;
    }
}
