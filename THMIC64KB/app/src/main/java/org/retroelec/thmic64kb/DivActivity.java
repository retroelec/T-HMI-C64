package org.retroelec.thmic64kb;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.Switch;
import androidx.appcompat.app.AppCompatActivity;

public class DivActivity extends AppCompatActivity {

    private SharedPreferences sharedPreferences;

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
        setContentView(R.layout.div);

        final Button toggleDrawEvenOddButton = findViewById(R.id.toggleDrawEvenOdd);
        toggleDrawEvenOddButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendCmd(DivActivity.this, new byte[]{(byte) 21, (byte) 0x00, (byte) 0x80});
            }
        });

        final Button toggleCIA2Button = findViewById(R.id.toggleCIA2);
        toggleCIA2Button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendCmd(DivActivity.this, new byte[]{(byte) 22, (byte) 0x00, (byte) 0x80});
            }
        });

        final Button resetButton = findViewById(R.id.reset);
        resetButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendCmd(DivActivity.this, new byte[]{(byte) 20, (byte) 0x00, (byte) 0x80});
            }
        });

        final Button quitButton = findViewById(R.id.close);
        quitButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
    }
}
