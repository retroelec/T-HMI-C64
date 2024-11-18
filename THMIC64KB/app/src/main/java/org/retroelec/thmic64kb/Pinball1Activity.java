package org.retroelec.thmic64kb;

import android.annotation.SuppressLint;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;

import androidx.appcompat.app.AppCompatActivity;

public class Pinball1Activity extends AppCompatActivity {
    private MyApplication myApplication;
    private Settings settings;

    private boolean saveSendRawKeyCodes;
    private boolean newSendRawKeyCodes;
    private boolean saveDetectReleaseKey;
    private boolean newDetectReleaseKey;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        myApplication = (MyApplication) this.getApplicationContext();
        settings = myApplication.getSettings();
        BLEUtils bleUtils = new BLEUtils(this);
        setContentView(R.layout.pinball1);
        setTitle("Pinball1");

        // send raw key codes and detect key release
        saveSendRawKeyCodes = settings.isSendRawKeyCodes();
        newSendRawKeyCodes = saveSendRawKeyCodes;
        if (!saveSendRawKeyCodes) {
            newSendRawKeyCodes = !newSendRawKeyCodes;
            bleUtils.send(new byte[]{Config.SENDRAWKEYS, (byte) 0x00, (byte) 0x80}, true);
        }
        saveDetectReleaseKey = settings.isDetectReleaseKey();
        newDetectReleaseKey = saveDetectReleaseKey;
        if (!saveDetectReleaseKey) {
            newDetectReleaseKey = !newDetectReleaseKey;
            bleUtils.send(new byte[]{Config.SWITCHDETECTRELEASEKEY, (byte) 0x00, (byte) 0x80}, true);
        }

        byte[] commodore = new byte[]{(byte) 0x7f, (byte) 0xdf, (byte) 0x00};
        final ImageButton flipperleftButton = findViewById(R.id.flipperleft);
        flipperleftButton.setOnTouchListener(bleUtils.createButtonTouchListener(
                flipperleftButton, commodore
        ));
        byte[] shiftright = new byte[]{(byte) 0xbf, (byte) 0xef, (byte) 0x00};
        final ImageButton flipperrightButton = findViewById(R.id.flipperright);
        flipperrightButton.setOnTouchListener(bleUtils.createButtonTouchListener(
                flipperrightButton, shiftright
        ));

        final Button quitButton = findViewById(R.id.close);
        quitButton.setOnClickListener(v -> {
            if (newSendRawKeyCodes != saveSendRawKeyCodes) {
                bleUtils.send(new byte[]{Config.SENDRAWKEYS, (byte) 0x00, (byte) 0x80}, true);
            }
            if (newDetectReleaseKey != saveDetectReleaseKey) {
                bleUtils.send(new byte[]{Config.SWITCHDETECTRELEASEKEY, (byte) 0x00, (byte) 0x80}, true);
            }
            finish();
        });
    }
}
