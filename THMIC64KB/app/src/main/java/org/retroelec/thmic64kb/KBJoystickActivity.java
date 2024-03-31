package org.retroelec.thmic64kb;

import android.content.Context;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

public class KBJoystickActivity extends AppCompatActivity {

    private static final byte VIRTUALJOYSTICKLEFT_ACTIVATED = (byte)0x02;
    private static final byte VIRTUALJOYSTICKLEFT_DEACTIVATED = (byte)0x82;
    private static final byte VIRTUALJOYSTICKRIGHT_ACTIVATED = (byte)0x03;
    private static final byte VIRTUALJOYSTICKRIGHT_DEACTIVATED = (byte)0x83;
    private static final byte VIRTUALJOYSTICKUP_ACTIVATED = (byte)0x00;
    private static final byte VIRTUALJOYSTICKUP_DEACTIVATED = (byte)0x80;
    private static final byte VIRTUALJOYSTICKDOWN_ACTIVATED = (byte)0x01;
    private static final byte VIRTUALJOYSTICKDOWN_DEACTIVATED = (byte) 0x81;
    private static final byte VIRTUALJOYSTICKFIRE_ACTIVATED = (byte) 0x04;
    private static final byte VIRTUALJOYSTICKFIRE_DEACTIVATED = (byte) 0x84;

    private void send(Context context, byte data) {
        byte[] datatosend = new byte[]{data};
        final Globals globals = (Globals) context.getApplicationContext();
        BLEManager bleManager = globals.getBleManager();
        if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
            bleManager.sendData(datatosend);
        }
    }

    private void sendCmd(Context context, byte[] data) {
        final Globals globals = (Globals) context.getApplicationContext();
        BLEManager bleManager = globals.getBleManager();
        if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
            bleManager.sendData(data);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.kbjoystick);
        setTitle("Virtual Joystick");

        // arrow buttons
        final ImageButton arrowupButton = (ImageButton) findViewById(R.id.arrowup);
        arrowupButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View arg0, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    arrowupButton.setBackgroundColor(Color.RED);
                    send(KBJoystickActivity.this, VIRTUALJOYSTICKUP_ACTIVATED);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    arrowupButton.setBackgroundColor(Color.TRANSPARENT);
                    send(KBJoystickActivity.this, VIRTUALJOYSTICKUP_DEACTIVATED);
                }
                return true;
            }
        });
        final ImageButton arrowdownButton = (ImageButton) findViewById(R.id.arrowdown);
        arrowdownButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View arg0, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    arrowdownButton.setBackgroundColor(Color.RED);
                    send(KBJoystickActivity.this, VIRTUALJOYSTICKDOWN_ACTIVATED);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    arrowdownButton.setBackgroundColor(Color.TRANSPARENT);
                    send(KBJoystickActivity.this, VIRTUALJOYSTICKDOWN_DEACTIVATED);
                }
                return true;
            }
        });
        final ImageButton arrowleftButton = (ImageButton) findViewById(R.id.arrowleft);
        arrowleftButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View arg0, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    arrowleftButton.setBackgroundColor(Color.RED);
                    send(KBJoystickActivity.this, VIRTUALJOYSTICKLEFT_ACTIVATED);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    arrowleftButton.setBackgroundColor(Color.TRANSPARENT);
                    send(KBJoystickActivity.this, VIRTUALJOYSTICKLEFT_DEACTIVATED);
                }
                return true;
            }
        });
        final ImageButton arrowrightButton = (ImageButton) findViewById(R.id.arrowright);
        arrowrightButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View arg0, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    arrowrightButton.setBackgroundColor(Color.RED);
                    send(KBJoystickActivity.this, VIRTUALJOYSTICKRIGHT_ACTIVATED);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    arrowrightButton.setBackgroundColor(Color.TRANSPARENT);
                    send(KBJoystickActivity.this, VIRTUALJOYSTICKRIGHT_DEACTIVATED);
                }
                return true;
            }
        });
        final ImageButton fireButton = (ImageButton) findViewById(R.id.fire);
        fireButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View arg0, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    fireButton.setBackgroundColor(Color.GREEN);
                    send(KBJoystickActivity.this, VIRTUALJOYSTICKFIRE_ACTIVATED);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    fireButton.setBackgroundColor(Color.TRANSPARENT);
                    send(KBJoystickActivity.this, VIRTUALJOYSTICKFIRE_DEACTIVATED);
                }
                return true;
            }
        });

        final Button quitButton = (Button) findViewById(R.id.close);
        quitButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendCmd(KBJoystickActivity.this, new byte[]{(byte) 6, (byte) 0x00, (byte) 0x80});
                finish();
            }
        });

    }
}
