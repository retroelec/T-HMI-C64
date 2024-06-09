package org.retroelec.thmic64kb;

import android.annotation.SuppressLint;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;

import androidx.appcompat.app.AppCompatActivity;

public class KBJoystickActivity extends AppCompatActivity {

    private void send(byte data) {
        byte[] datatosend = new byte[]{data};
        final MyApplication myApplication = (MyApplication) getApplication();
        BLEManager bleManager = myApplication.getBleManager();
        if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
            bleManager.sendData(datatosend);
        }
    }

    private void sendCmd(byte[] data) {
        final MyApplication myApplication = (MyApplication) getApplication();
        BLEManager bleManager = myApplication.getBleManager();
        if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
            bleManager.sendData(data);
        }
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.kbjoystick);
        setTitle("Virtual Joystick");

        // arrow buttons
        final ImageButton arrowupButton = findViewById(R.id.arrowup);
        arrowupButton.setOnTouchListener((arg0, event) -> {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                Log.d("THMIC64", "up button");
                arrowupButton.setBackgroundColor(Color.RED);
                send(Config.VIRTUALJOYSTICKUP_ACTIVATED);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                arrowupButton.setBackgroundColor(Color.TRANSPARENT);
                send(Config.VIRTUALJOYSTICKUP_DEACTIVATED);
            }
            return true;
        });
        final ImageButton arrowdownButton = findViewById(R.id.arrowdown);
        arrowdownButton.setOnTouchListener((arg0, event) -> {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                Log.d("THMIC64", "down button");
                arrowdownButton.setBackgroundColor(Color.RED);
                send(Config.VIRTUALJOYSTICKDOWN_ACTIVATED);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                arrowdownButton.setBackgroundColor(Color.TRANSPARENT);
                send(Config.VIRTUALJOYSTICKDOWN_DEACTIVATED);
            }
            return true;
        });
        final ImageButton arrowleftButton = findViewById(R.id.arrowleft);
        arrowleftButton.setOnTouchListener((arg0, event) -> {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                Log.d("THMIC64", "left button");
                arrowleftButton.setBackgroundColor(Color.RED);
                send(Config.VIRTUALJOYSTICKLEFT_ACTIVATED);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                arrowleftButton.setBackgroundColor(Color.TRANSPARENT);
                send(Config.VIRTUALJOYSTICKLEFT_DEACTIVATED);
            }
            return true;
        });
        final ImageButton arrowrightButton = findViewById(R.id.arrowright);
        arrowrightButton.setOnTouchListener((arg0, event) -> {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                Log.d("THMIC64", "right button");
                arrowrightButton.setBackgroundColor(Color.RED);
                send(Config.VIRTUALJOYSTICKRIGHT_ACTIVATED);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                arrowrightButton.setBackgroundColor(Color.TRANSPARENT);
                send(Config.VIRTUALJOYSTICKRIGHT_DEACTIVATED);
            }
            return true;
        });
        final ImageButton fire1Button = findViewById(R.id.fire1);
        fire1Button.setOnTouchListener((arg0, event) -> {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                Log.d("THMIC64", "button1");
                fire1Button.setBackgroundColor(Color.GREEN);
                send(Config.VIRTUALJOYSTICKFIRE_ACTIVATED);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                fire1Button.setBackgroundColor(Color.TRANSPARENT);
                send(Config.VIRTUALJOYSTICKFIRE_DEACTIVATED);
            }
            return true;
        });
        final ImageButton fire2Button = findViewById(R.id.fire2);
        fire2Button.setOnTouchListener((arg0, event) -> {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                Log.d("THMIC64", "button2");
                fire2Button.setBackgroundColor(Color.GREEN);
                send(Config.VIRTUALJOYSTICKFIRE_ACTIVATED);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                fire2Button.setBackgroundColor(Color.TRANSPARENT);
                send(Config.VIRTUALJOYSTICKFIRE_DEACTIVATED);
            }
            return true;
        });
        boolean showFire2Button = getIntent().getBooleanExtra("SHOW_FIRE2_BUTTON", false);
        if (showFire2Button) {
            fire2Button.setVisibility(View.VISIBLE);
        } else {
            fire2Button.setVisibility(View.GONE);
        }
        final Button quitButton = findViewById(R.id.close);
        quitButton.setOnClickListener(v -> {
            sendCmd(new byte[]{Config.KBJOYSTICKMODEOFF, (byte) 0x00, (byte) 0x80});
            finish();
        });
    }
}
