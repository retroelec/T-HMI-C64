package org.retroelec.thmic64kb;

import android.os.Bundle;
import android.text.InputFilter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class MemoryActivity extends AppCompatActivity implements NotificationObserver {
    private Settings settings;
    private Type3Notification type3Notification;
    private EditText address;
    private TextView val1, val2, val3, val4, val5, val6, val7, val8, val9, val10, val11, val12, val13, val14, val15, val16;

    @Override
    public void update() {
        short[] mem = type3Notification.getMem();
        if (mem != null && mem.length == 16) {
            val1.setText(String.format("%02X", mem[0] & 0xFF));
            val2.setText(String.format("%02X", mem[1] & 0xFF));
            val3.setText(String.format("%02X", mem[2] & 0xFF));
            val4.setText(String.format("%02X", mem[3] & 0xFF));
            val5.setText(String.format("%02X", mem[4] & 0xFF));
            val6.setText(String.format("%02X", mem[5] & 0xFF));
            val7.setText(String.format("%02X", mem[6] & 0xFF));
            val8.setText(String.format("%02X", mem[7] & 0xFF));
            val9.setText(String.format("%02X", mem[8] & 0xFF));
            val10.setText(String.format("%02X", mem[9] & 0xFF));
            val11.setText(String.format("%02X", mem[10] & 0xFF));
            val12.setText(String.format("%02X", mem[11] & 0xFF));
            val13.setText(String.format("%02X", mem[12] & 0xFF));
            val14.setText(String.format("%02X", mem[13] & 0xFF));
            val15.setText(String.format("%02X", mem[14] & 0xFF));
            val16.setText(String.format("%02X", mem[15] & 0xFF));
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final MyApplication myApplication = (MyApplication) getApplication();
        settings = myApplication.getSettings();
        type3Notification = myApplication.getType3Notification();
        type3Notification.registerObserver(this);

        setContentView(R.layout.memory);

        address = findViewById(R.id.address);
        address.setFilters(new InputFilter[]{
                (source, start, end, dest, dstart, dend) -> {
                    for (int i = start; i < end; i++) {
                        char ch = source.charAt(i);
                        if (!Character.isDigit(ch) && (ch < 'A' || ch > 'F') && (ch < 'a' || ch > 'f')) {
                            return "";
                        }
                    }
                    return null;
                }
        });

        final Button getMemoryButton = findViewById(R.id.getMemory);
        getMemoryButton.setOnClickListener(v -> {
            String addr = address.getText().toString().trim();
            if (addr.length() > 4) {
                Toast.makeText(this, "address must be a 16 bit hex value", Toast.LENGTH_LONG).show();
            }
            try {
                int val = Integer.parseInt(addr, 16);
                BLEManager bleManager = myApplication.getBleManager();
                if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
                    bleManager.sendData(new byte[]{Config.SHOWMEM, (byte) 0x00, (byte) 0x80, (byte) val, (byte) (val >> 8)}, false);
                }
            } catch (NumberFormatException e) {
                Toast.makeText(this, "address must be a 16 bit hex value", Toast.LENGTH_LONG).show();
            }
        });

        val1 = findViewById(R.id.val1);
        val2 = findViewById(R.id.val2);
        val3 = findViewById(R.id.val3);
        val4 = findViewById(R.id.val4);
        val5 = findViewById(R.id.val5);
        val6 = findViewById(R.id.val6);
        val7 = findViewById(R.id.val7);
        val8 = findViewById(R.id.val8);
        val9 = findViewById(R.id.val9);
        val10 = findViewById(R.id.val10);
        val11 = findViewById(R.id.val11);
        val12 = findViewById(R.id.val12);
        val13 = findViewById(R.id.val13);
        val14 = findViewById(R.id.val14);
        val15 = findViewById(R.id.val15);
        val16 = findViewById(R.id.val16);

        final Button quitButton = findViewById(R.id.close);
        quitButton.setOnClickListener(v -> finish());
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        type3Notification.removeObserver();
    }
}
