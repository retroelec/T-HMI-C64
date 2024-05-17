package org.retroelec.thmic64kb;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Switch;

import androidx.appcompat.app.AppCompatActivity;

public class DivActivity extends AppCompatActivity {
    private Switch toggleRefreshframecolorSwitch;
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

        toggleRefreshframecolorSwitch = findViewById(R.id.toggleRefreshframecolor);
        toggleRefreshframecolorSwitch.setChecked(true);
        toggleRefreshframecolorSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) {
                toggleRefreshframecolorSwitch.setChecked(true);
            } else {
                toggleRefreshframecolorSwitch.setChecked(false);
            }
            sendCmd(DivActivity.this, new byte[]{(byte) 22, (byte) 0x00, (byte) 0x80});
        });

        final Switch toggleCIA2Button = findViewById(R.id.toggleCIA2);
        toggleCIA2Button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendCmd(DivActivity.this, new byte[]{(byte) 23, (byte) 0x00, (byte) 0x80});
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
