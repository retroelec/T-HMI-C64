package org.retroelec.thmic64kb;

public class Type3Notification {
    private NotificationObserver observer = null;
    private short[] mem;

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

    public short[] getMem() {
        return mem;
    }

    public void setMem(short[] mem) {
        this.mem = mem;
    }
}
