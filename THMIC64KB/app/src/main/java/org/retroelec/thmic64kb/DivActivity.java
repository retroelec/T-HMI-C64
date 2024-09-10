package org.retroelec.thmic64kb;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.Switch;

import androidx.appcompat.app.AppCompatActivity;

public class DivActivity extends AppCompatActivity implements SettingsObserver {
    private Switch toggleRefreshframecolorSwitch;
    private Switch toggleSendRawKeyCodes;
    private Switch toggleDebug;
    private Switch togglePerf;
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
        runOnUiThread(() -> {
            toggleRefreshframecolorSwitch.setChecked(settings.isRefreshframecolor());
            toggleSendRawKeyCodes.setChecked(settings.isSendRawKeyCodes());
            toggleDebug.setChecked(settings.isDebug());
            togglePerf.setChecked(settings.isPerf());
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

        toggleSendRawKeyCodes = findViewById(R.id.toggleSendRawKeyCodes);
        toggleSendRawKeyCodes.setOnClickListener(v -> sendCmd(new byte[]{Config.SENDRAWKEYS, (byte) 0x00, (byte) 0x80}));

        toggleDebug = findViewById(R.id.toggleDebug);
        toggleDebug.setOnClickListener(v -> sendCmd(new byte[]{Config.SWITCHDEBUG, (byte) 0x00, (byte) 0x80}));

        togglePerf = findViewById(R.id.togglePerf);
        togglePerf.setOnClickListener(v -> sendCmd(new byte[]{Config.SWITCHPERF, (byte) 0x00, (byte) 0x80}));

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
