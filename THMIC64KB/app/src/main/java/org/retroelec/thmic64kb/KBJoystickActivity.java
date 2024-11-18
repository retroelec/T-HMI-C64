package org.retroelec.thmic64kb;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;

import androidx.appcompat.app.AppCompatActivity;

public class KBJoystickActivity extends AppCompatActivity {
    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        BLEUtils bleUtils = new BLEUtils(this);
        setContentView(R.layout.kbjoystick);
        setTitle("Virtual Joystick");

        final ImageButton arrowupButton = findViewById(R.id.arrowup);
        final ImageButton arrowdownButton = findViewById(R.id.arrowdown);
        final ImageButton arrowleftButton = findViewById(R.id.arrowleft);
        final ImageButton arrowrightButton = findViewById(R.id.arrowright);
        final ImageButton fire1Button = findViewById(R.id.fire1);
        final ImageButton fire2Button = findViewById(R.id.fire2);
        arrowupButton.setOnTouchListener(bleUtils.createButtonTouchListener(
                arrowupButton,
                Config.VIRTUALJOYSTICKUP_ACTIVATED,
                Config.VIRTUALJOYSTICKUP_DEACTIVATED
        ));
        arrowdownButton.setOnTouchListener(bleUtils.createButtonTouchListener(
                arrowdownButton,
                Config.VIRTUALJOYSTICKDOWN_ACTIVATED,
                Config.VIRTUALJOYSTICKDOWN_DEACTIVATED
        ));
        arrowleftButton.setOnTouchListener(bleUtils.createButtonTouchListener(
                arrowleftButton,
                Config.VIRTUALJOYSTICKLEFT_ACTIVATED,
                Config.VIRTUALJOYSTICKLEFT_DEACTIVATED
        ));
        arrowrightButton.setOnTouchListener(bleUtils.createButtonTouchListener(
                arrowrightButton,
                Config.VIRTUALJOYSTICKRIGHT_ACTIVATED,
                Config.VIRTUALJOYSTICKRIGHT_DEACTIVATED
        ));

        fire1Button.setOnTouchListener(bleUtils.createButtonTouchListener(
                fire1Button,
                Config.VIRTUALJOYSTICKFIRE_ACTIVATED,
                Config.VIRTUALJOYSTICKFIRE_DEACTIVATED
        ));
        fire2Button.setOnTouchListener(bleUtils.createButtonTouchListener(
                fire2Button, new byte[]{(byte) 0x7f, (byte) 0xef, (byte) 0x00}
        ));

        boolean showFire2Button = getIntent().getBooleanExtra("SHOW_FIRE2_BUTTON", false);
        if (showFire2Button) {
            fire2Button.setVisibility(View.VISIBLE);
        } else {
            fire2Button.setVisibility(View.GONE);
        }
        final Button quitButton = findViewById(R.id.close);
        quitButton.setOnClickListener(v -> {
            bleUtils.send(new byte[]{Config.KBJOYSTICKMODEOFF, (byte) 0x00, (byte) 0x80});
            finish();
        });
    }
}
