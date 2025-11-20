package org.retroelec.thmic64kb;

import java.util.ArrayList;
import java.util.List;

public class Settings {
    private List<SettingsObserver> observerList = new ArrayList<>();
    private byte joymode;
    private int volume;
    private boolean d64attached;
    private boolean deactivateTemp;
    private boolean sendRawKeyCodes;
    private boolean debug;
    private boolean perf;
    private boolean detectReleaseKey;
    private short minKeyPressedDuration;
    private boolean powerOff;

    public void registerSettingsObserver(SettingsObserver observer) {
        this.observerList.add(observer);
    }

    public void notifySettingsObserver() {
        for (SettingsObserver observer : observerList) {
            observer.updateSettings();
        }
    }

    public void removeSettingsObserver() {
        observerList.clear();
    }

    public byte getJoymode() {
        return joymode;
    }

    public void setJoymode(byte joymode) {
        this.joymode = joymode;
    }

    public int getVolume() {
        return volume;
    }

    public void setVolume(int volume) {
        this.volume = volume;
    }

    public boolean isd64attached() {
        return d64attached;
    }

    public void setD64attached(boolean d64attached) {
        this.d64attached = d64attached;
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
