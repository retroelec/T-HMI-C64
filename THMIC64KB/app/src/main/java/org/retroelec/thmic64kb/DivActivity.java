package org.retroelec.thmic64kb;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Switch;

import androidx.appcompat.app.AppCompatActivity;

public class DivActivity extends AppCompatActivity implements SettingsObserver {
    private Switch toggleRefreshframecolorSwitch;
    private Switch toggleSendRawKeyCodes;
    private Switch toggleDebug;
    private Switch togglePerf;
    private Switch toggleDetectReleaseKey;
    private EditText inputMinKeyPressedDuration;
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
            toggleDetectReleaseKey.setChecked(settings.isDetectReleaseKey());
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

        toggleDetectReleaseKey = findViewById(R.id.toggleDetectReleaseKey);
        toggleDetectReleaseKey.setOnClickListener(v -> sendCmd(new byte[]{Config.SWITCHDETECTRELEASEKEY, (byte) 0x00, (byte) 0x80}));

        inputMinKeyPressedDuration = findViewById(R.id.inputMinKeyPressedDuration);
        inputMinKeyPressedDuration.setText(String.valueOf(settings.getMinKeyPressedDuration()));
        inputMinKeyPressedDuration.setOnEditorActionListener((v, actionId, event) -> {
            if (actionId == EditorInfo.IME_ACTION_DONE || actionId == EditorInfo.IME_ACTION_NEXT || event != null && event.getKeyCode() == KeyEvent.KEYCODE_ENTER) {
                String input = inputMinKeyPressedDuration.getText().toString();
                try {
                    int value = Integer.parseInt(input);
                    if (value < 120) {
                        inputMinKeyPressedDuration.setText("120");
                    } else if (value > 300) {
                        inputMinKeyPressedDuration.setText("300");
                    }
                } catch (NumberFormatException e) {
                    inputMinKeyPressedDuration.setText(String.valueOf(Config.DEFAULT_MINKEYPRESSEDDURATION));
                }
                settings.setMinKeyPressedDuration(Short.parseShort(String.valueOf(inputMinKeyPressedDuration.getText())));
                inputMinKeyPressedDuration.clearFocus();
                InputMethodManager imm = (InputMethodManager) inputMinKeyPressedDuration.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(inputMinKeyPressedDuration.getWindowToken(), 0);
                return true;
            }
            return false;
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
