package org.retroelec.thmic64kb;

public class Settings {
    private SettingsObserver observer = null;
    byte joymode;
    boolean refreshframecolor;
    boolean sendRawKeyCodes;
    boolean debug;
    boolean perf;

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

    public boolean isSendRawKeyCodes() {
        return sendRawKeyCodes;
    }

    public void setSendRawKeyCodes(boolean sendRawKeyCodes) {
        this.sendRawKeyCodes = sendRawKeyCodes;
    }

    public boolean isDebug() {
        return debug;
    }

    public void setDebug(boolean debug) {
        this.debug = debug;
    }

    public boolean isPerf() {
        return perf;
    }

    public void setPerf(boolean perf) {
        this.perf = perf;
    }
}
