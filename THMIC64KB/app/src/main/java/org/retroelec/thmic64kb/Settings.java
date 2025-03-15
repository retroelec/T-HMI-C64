package org.retroelec.thmic64kb;

public class Settings {
    private SettingsObserver observer = null;
    byte joymode;
    boolean deactivateTemp;
    boolean sendRawKeyCodes;
    boolean debug;
    boolean perf;
    boolean detectReleaseKey;
    short minKeyPressedDuration;
    boolean powerOff;

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

    public boolean isDeactivateTemp() {
        return deactivateTemp;
    }

    public void setDeactivateTemp(boolean deactivateTemp) {
        this.deactivateTemp = deactivateTemp;
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

    public boolean isDetectReleaseKey() {
        return detectReleaseKey;
    }

    public void setDetectReleaseKey(boolean detectReleaseKey) {
        this.detectReleaseKey = detectReleaseKey;
    }

    public short getMinKeyPressedDuration() {
        return minKeyPressedDuration;
    }

    public void setMinKeyPressedDuration(short minKeyPressedDuration) {
        this.minKeyPressedDuration = minKeyPressedDuration;
    }

    public boolean isPowerOff() {
        return powerOff;
    }

    public void setPowerOff(boolean powerOff) {
        this.powerOff = powerOff;
    }
}
