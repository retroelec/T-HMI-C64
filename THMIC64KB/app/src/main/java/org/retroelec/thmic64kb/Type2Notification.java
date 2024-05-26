package org.retroelec.thmic64kb;

public class Type2Notification {
    private Type2NotificationObserver observer = null;
    private boolean cpuRunning;
    private int pc;
    private short a;
    private short x;
    private short y;
    private short sr;
    private short d011;
    private short d016;
    private short d018;
    private short d019;
    private short d01a;
    private short register1;

    public void registerObserver(Type2NotificationObserver observer) {
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

    public boolean isCpuRunning() {
        return cpuRunning;
    }

    public void setCpuRunning(boolean cpuRunning) {
        this.cpuRunning = cpuRunning;
    }

    public int getPc() {
        return pc;
    }

    public void setPc(int pc) {
        this.pc = pc;
    }

    public short getA() {
        return a;
    }

    public void setA(short a) {
        this.a = a;
    }

    public short getX() {
        return x;
    }

    public void setX(short x) {
        this.x = x;
    }

    public short getY() {
        return y;
    }

    public void setY(short y) {
        this.y = y;
    }

    public short getSr() {
        return sr;
    }

    public void setSr(short sr) {
        this.sr = sr;
    }

    public short getD011() {
        return d011;
    }

    public void setD011(short d011) {
        this.d011 = d011;
    }

    public short getD016() {
        return d016;
    }

    public void setD016(short d016) {
        this.d016 = d016;
    }

    public short getD018() {
        return d018;
    }

    public void setD018(short d018) {
        this.d018 = d018;
    }

    public short getD019() {
        return d019;
    }

    public void setD019(short d019) {
        this.d019 = d019;
    }

    public short getD01a() {
        return d01a;
    }

    public void setD01a(short d01a) {
        this.d01a = d01a;
    }

    public short getRegister1() {
        return register1;
    }

    public void setRegister1(short register1) {
        this.register1 = register1;
    }
}
