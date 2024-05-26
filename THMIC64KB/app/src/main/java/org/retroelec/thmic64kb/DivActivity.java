package org.retroelec.thmic64kb;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.Switch;

import androidx.appcompat.app.AppCompatActivity;

public class DivActivity extends AppCompatActivity implements SettingsObserver {
    private Switch toggleRefreshframecolorSwitch;
    private Switch toggleCIA2Switch;
    private Spinner joyEmulMode;
    private Settings settings;

    private void sendCmd(byte[] data) {
        final MyApplication myApplication = (MyApplication) getApplication();
        BLEManager bleManager = myApplication.getBleManager();
        if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
            bleManager.sendData(data);
        }
    }

    @Override
    public void updateSettings() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                toggleRefreshframecolorSwitch.setChecked(settings.isRefreshframecolor());
                toggleCIA2Switch.setChecked(!settings.isDeactivatecia2());
                joyEmulMode.setSelection(settings.getJoyemulmode());
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        settings.registerSettingsObserver(this);
        updateSettings();
    }

    @Override
    protected void onPause() {
        super.onPause();
        settings.removeSettingsObserver();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final MyApplication myApplication = (MyApplication) getApplication();
        settings = myApplication.getSettings();

        setContentView(R.layout.div);

        toggleRefreshframecolorSwitch = findViewById(R.id.toggleRefreshframecolor);
        toggleRefreshframecolorSwitch.setOnClickListener(v -> sendCmd(new byte[]{Config.SWITCHFRAMECOLORREFRESH, (byte) 0x00, (byte) 0x80}));

        toggleCIA2Switch = findViewById(R.id.toggleCIA2);
        toggleCIA2Switch.setOnClickListener(v -> sendCmd(new byte[]{Config.SWITCHCIA2, (byte) 0x00, (byte) 0x80}));

        joyEmulMode = findViewById(R.id.joyemulmode);
        String[] joyEmulArray = new String[]{"JoyEmul1", "JoyEmul2", "JoyEmul3"};
        ArrayAdapter adapter = new ArrayAdapter(this, android.R.layout.simple_spinner_item, joyEmulArray);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        joyEmulMode.setAdapter(adapter);
        joyEmulMode.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                byte joyemul;
                switch (parent.getItemAtPosition(position).toString()) {
                    case "JoyEmul1":
                        joyemul = 0;
                        break;
                    case "JoyEmul2":
                        joyemul = 1;
                        break;
                    case "JoyEmul3":
                        joyemul = 2;
                        break;
                    default:
                        joyemul = 0;
                        break;
                }
                sendCmd(new byte[]{Config.JOYEMULMODE, (byte) 0x00, (byte) 0x80, joyemul});
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        final Button keystatus = findViewById(R.id.keystatus);
        keystatus.setOnClickListener(view -> {
            Intent i = new Intent(this, StatusActivity.class);
            startActivity(i);
        });

        final Button keymemory = findViewById(R.id.keymemory);
        keymemory.setOnClickListener(view -> {
            Intent i = new Intent(this, MemoryActivity.class);
            startActivity(i);
        });

        final Button resetButton = findViewById(R.id.reset);
        resetButton.setOnClickListener(v -> sendCmd(new byte[]{Config.RESET, (byte) 0x00, (byte) 0x80}));

        final Button closeButton = findViewById(R.id.close);
        closeButton.setOnClickListener(v -> finish());

        updateSettings();
    }
}
