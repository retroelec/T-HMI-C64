package org.retroelec.thmic64kb;

public class Type6Notification {
    private NotificationObserver observer = null;
    private int version;

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

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }
}