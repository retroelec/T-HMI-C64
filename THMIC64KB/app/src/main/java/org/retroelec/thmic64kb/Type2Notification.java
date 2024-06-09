package org.retroelec.thmic64kb;

public class Type2Notification {
    private NotificationObserver observer = null;
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
    private short dc0d;
    private short dc0e;
    private short dc0f;
    private short dd0d;
    private short dd0e;
    private short dd0f;

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

    public short getDc0d() {
        return dc0d;
    }

    public void setDc0d(short dc0d) {
        this.dc0d = dc0d;
    }

    public short getDc0e() {
        return dc0e;
    }

    public void setDc0e(short dc0e) {
        this.dc0e = dc0e;
    }

    public short getDc0f() {
        return dc0f;
    }

    public void setDc0f(short dc0f) {
        this.dc0f = dc0f;
    }

    public short getDd0d() {
        return dd0d;
    }

    public void setDd0d(short dd0d) {
        this.dd0d = dd0d;
    }

    public short getDd0e() {
        return dd0e;
    }

    public void setDd0e(short dd0e) {
        this.dd0e = dd0e;
    }

    public short getDd0f() {
        return dd0f;
    }

    public void setDd0f(short dd0f) {
        this.dd0f = dd0f;
    }
}
