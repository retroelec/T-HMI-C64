package org.retroelec.thmic64kb;

import android.os.Bundle;
import android.widget.Button;
import android.widget.Switch;

import androidx.appcompat.app.AppCompatActivity;

public class MemoryActivity extends AppCompatActivity {
    private Settings settings;

    private void sendCmd(byte[] data) {
        final MyApplication myApplication = (MyApplication) getApplication();
        BLEManager bleManager = myApplication.getBleManager();
        if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
            bleManager.sendData(data);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final MyApplication myApplication = (MyApplication) getApplication();
        settings = myApplication.getSettings();

        setContentView(R.layout.memory);

        final Button quitButton = findViewById(R.id.close);
        quitButton.setOnClickListener(v -> finish());
    }
}
