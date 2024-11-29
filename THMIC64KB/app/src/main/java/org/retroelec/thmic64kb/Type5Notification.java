package org.retroelec.thmic64kb;

public class Type5Notification {
    private NotificationObserver observer = null;
    private int batteryVoltage;

    public void registerObserver(NotificationObserver observer) {
        this.observer = observer;
    }

    public void notifyObserver() {
        if (observer != null) {
            observer.update();
        }
    }

    public void removeObserver() {
        observer = null;
    }

    public int getBatteryVoltage() {
        return batteryVoltage;
    }

    public void setBatteryVoltage(int batteryVoltage) {
        this.batteryVoltage = batteryVoltage;
    }
}
