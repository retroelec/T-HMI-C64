package org.retroelec.thmic64kb;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;

import androidx.appcompat.app.AppCompatActivity;

public class KBInpActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.kbinp);
        BLEUtils bleUtils = new BLEUtils(this);

        final Button keykbjoystick1 = findViewById(R.id.keykbjoystick1);
        keykbjoystick1.setOnClickListener(view -> {
            bleUtils.send(new byte[]{Config.KBJOYSTICKMODE1, (byte) 0x00, (byte) 0x80}, false);
            Intent i = new Intent(this, KBJoystickActivity.class);
            startActivity(i);
        });

        final Button keykbjoystick2 = findViewById(R.id.keykbjoystick2);
        keykbjoystick2.setOnClickListener(view -> {
            bleUtils.send(new byte[]{Config.KBJOYSTICKMODE2, (byte) 0x00, (byte) 0x80}, false);
            Intent i = new Intent(this, KBJoystickActivity.class);
            i.putExtra("SHOW_FIRE2_BUTTON", true);
            startActivity(i);
        });

        final Button keykbpinball1 = findViewById(R.id.keykbpinball1);
        keykbpinball1.setOnClickListener(view -> {
            Intent i = new Intent(this, Pinball1Activity.class);
            startActivity(i);
        });

        final Button closeButton = findViewById(R.id.close);
        closeButton.setOnClickListener(v -> finish());
    }
}
