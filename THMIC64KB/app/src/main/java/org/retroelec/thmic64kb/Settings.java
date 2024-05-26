package org.retroelec.thmic64kb;

public class Settings {
    private SettingsObserver observer = null;
    byte joymode;
    boolean refreshframecolor;
    boolean deactivatecia2;
    byte joyemulmode;

    public void registerSettingsObserver(SettingsObserver observer) {
        this.observer = observer;
    }

    public void notifySettingsObserver() {
        if (observer != null) {
            observer.updateSettings();
        }
    }

    public void removeSettingsObserver() {
        observer = null;
    }

    public byte getJoymode() {
        return joymode;
    }

    public void setJoymode(byte joymode) {
        this.joymode = joymode;
    }

    public boolean isRefreshframecolor() {
        return refreshframecolor;
    }

    public void setRefreshframecolor(boolean refreshframecolor) {
        this.refreshframecolor = refreshframecolor;
    }

    public boolean isDeactivatecia2() {
        return deactivatecia2;
    }

    public void setDeactivatecia2(boolean deactivatecia2) {
        this.deactivatecia2 = deactivatecia2;
    }

    public byte getJoyemulmode() {
        return joyemulmode;
    }

    public void setJoyemulmode(byte joyemulmode) {
        this.joyemulmode = joyemulmode;
    }
}
