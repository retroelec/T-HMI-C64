package org.retroelec.thmic64kb;

public class Type4Notification {
    private NotificationObserver observer = null;

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
}
