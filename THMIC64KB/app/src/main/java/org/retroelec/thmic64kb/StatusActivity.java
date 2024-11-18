package org.retroelec.thmic64kb;

import android.os.Bundle;
import android.widget.Button;
import android.widget.Switch;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

public class StatusActivity extends AppCompatActivity implements NotificationObserver {
    private Type2Notification type2Notification;
    private Switch cpuRunningSwitch;
    private TextView reg1;
    private TextView pc;
    private TextView a;
    private TextView x;
    private TextView y;
    private TextView sr;
    private TextView d011;
    private TextView d016;
    private TextView d018;
    private TextView d019;
    private TextView d01a;
    private TextView dc0d;
    private TextView dc0e;
    private TextView dc0f;
    private TextView dd0d;
    private TextView dd0e;
    private TextView dd0f;

    @Override
    public void update() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                cpuRunningSwitch.setChecked(type2Notification.isCpuRunning());
                reg1.setText(String.format("%02X", type2Notification.getRegister1() & 0xFF));
                pc.setText(String.format("%04X", type2Notification.getPc() & 0xFFFF));
                a.setText(String.format("%02X", type2Notification.getA() & 0xFF));
                x.setText(String.format("%02X", type2Notification.getX() & 0xFF));
                y.setText(String.format("%02X", type2Notification.getY() & 0xFF));
                sr.setText(String.format("%02X", type2Notification.getSr() & 0xFF));
                d011.setText(String.format("%02X", type2Notification.getD011() & 0xFF));
                d016.setText(String.format("%02X", type2Notification.getD016() & 0xFF));
                d018.setText(String.format("%02X", type2Notification.getD018() & 0xFF));
                d019.setText(String.format("%02X", type2Notification.getD019() & 0xFF));
                d01a.setText(String.format("%02X", type2Notification.getD01a() & 0xFF));
                dc0d.setText(String.format("%02X", type2Notification.getDc0d() & 0xFF));
                dc0e.setText(String.format("%02X", type2Notification.getDc0e() & 0xFF));
                dc0f.setText(String.format("%02X", type2Notification.getDc0f() & 0xFF));
                dd0d.setText(String.format("%02X", type2Notification.getDd0d() & 0xFF));
                dd0e.setText(String.format("%02X", type2Notification.getDd0e() & 0xFF));
                dd0f.setText(String.format("%02X", type2Notification.getDd0f() & 0xFF));
            }
        });
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        BLEUtils bleUtils = new BLEUtils(this);
        final MyApplication myApplication = (MyApplication) getApplication();
        type2Notification = myApplication.getType2Notification();
        type2Notification.registerObserver(this);
        bleUtils.send(new byte[]{Config.SHOWREG, (byte) 0x00, (byte) 0x80}, false);

        setContentView(R.layout.status);

        cpuRunningSwitch = findViewById(R.id.cpuRunning);
        reg1 = findViewById(R.id.valReg1);
        pc = findViewById(R.id.valPc);
        a = findViewById(R.id.valA);
        x = findViewById(R.id.valX);
        y = findViewById(R.id.valY);
        sr = findViewById(R.id.valSr);
        d011 = findViewById(R.id.valD011);
        d016 = findViewById(R.id.valD016);
        d018 = findViewById(R.id.valD018);
        d019 = findViewById(R.id.valD019);
        d01a = findViewById(R.id.valD01A);
        dc0d = findViewById(R.id.valDC0D);
        dc0e = findViewById(R.id.valDC0E);
        dc0f = findViewById(R.id.valDC0F);
        dd0d = findViewById(R.id.valDD0D);
        dd0e = findViewById(R.id.valDD0E);
        dd0f = findViewById(R.id.valDD0F);

        final Button closeButton = findViewById(R.id.close);
        closeButton.setOnClickListener(v -> finish());
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        type2Notification.removeObserver();
    }
}
