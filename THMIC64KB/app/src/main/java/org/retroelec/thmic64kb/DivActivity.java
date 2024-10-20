package org.retroelec.thmic64kb;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.Switch;
import android.widget.Toast;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class DivActivity extends AppCompatActivity implements SettingsObserver, NotificationObserver {
    private Switch toggleRefreshframecolorSwitch;
    private Switch toggleSendRawKeyCodes;
    private Switch toggleDebug;
    private Switch togglePerf;
    private Switch toggleDetectReleaseKey;
    private EditText inputMinKeyPressedDuration;
    private Settings settings;
    private MyApplication myApplication;
    private Type4Notification type4Notification;
    private volatile boolean ackFromServer = false;

    private static final int PICK_FILE_REQUEST_CODE = 1;
    private ActivityResultLauncher<Intent> filePickerLauncher;
    private byte[] buffer = new byte[64 * 1024];
    private int bufferlen;

    private void sendCmd(byte[] data, boolean blocking) {
        BLEManager bleManager = myApplication.getBleManager();
        if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
            bleManager.sendData(data, blocking);
        }
    }

    private void openFilePicker() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.setType("application/octet-stream");
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        filePickerLauncher.launch(intent);
    }

    private void sendNextBlock(int blocknr, boolean firstBlock, boolean lastBlock, int lengthLastBlock) {
        int start = blocknr * 250;
        byte[] block = new byte[254];
        byte blockdetail = 0;
        int headerlen = 3;
        int datalen = 250;
        if (firstBlock) {
            blockdetail = 1;
        } else if (lastBlock) {
            blockdetail = 2;
            headerlen = 4; // add one byte for length info in last block
            datalen = lengthLastBlock;
        }
        if (lastBlock) {
            System.arraycopy(new byte[]{Config.RECEIVEDATA, blockdetail, (byte) 0x80, (byte) datalen}, 0, block, 0, headerlen);
        } else {
            System.arraycopy(new byte[]{Config.RECEIVEDATA, blockdetail, (byte) 0x80}, 0, block, 0, headerlen);
        }
        System.arraycopy(buffer, start, block, headerlen, datalen);
        Log.i("THMIC64", "sendNextBlock, detail = " + blockdetail);
        sendCmd(block, true);
    }

    private void wait4Ack() {
        long startTime = System.currentTimeMillis();
        long timeout = 3000; // 3000ms
        while (!ackFromServer) {
            if (System.currentTimeMillis() - startTime > timeout) {
                Log.i("THMIC64", "wait4Ack timeout!!");
                break;
            }
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        ackFromServer = false;
        long duration = System.currentTimeMillis() - startTime;
        Log.i("THMIC64", "wait4Ack received data after " + duration);
    }

    private void readFileFromUri(Context context, Uri uri) throws IOException {
        InputStream inputStream = context.getContentResolver().openInputStream(uri);
        ByteArrayOutputStream byteBuffer = new ByteArrayOutputStream();
        int len;
        while ((len = inputStream.read(buffer)) != -1) {
            byteBuffer.write(buffer, 0, len);
        }
        bufferlen = byteBuffer.size();
        inputStream.close();
    }

    private AlertDialog progressDialog;
    private ProgressBar progressBar;
    private boolean isCancelled;

    private void showProgressDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("transfer in progress...")
                .setMessage("data will be sent...")
                .setCancelable(false) // dialog may not be closed using the back button
                .setNegativeButton("Cancel", (dialog, which) -> {
                    isCancelled = true;
                    dismissProgressDialog();
                });
        progressBar = new ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal);
        progressBar.setMax(100);
        builder.setView(progressBar);
        progressDialog = builder.create();
        progressDialog.show();
    }

    private void updateProgress(int actblock, int numOfBlocks) {
        int percent = 100 * actblock / numOfBlocks;
        if (progressBar != null) {
            progressBar.setProgress(percent);
        }
        if (progressDialog != null && progressDialog.isShowing()) {
            progressDialog.setMessage(percent + "%");
        }
    }

    private void dismissProgressDialog() {
        if (progressDialog != null && progressDialog.isShowing()) {
            progressDialog.dismiss();
        }
    }

    private void startTransfer(ActivityResult result) {
        isCancelled = false;
        ackFromServer = false;
        Uri uri = result.getData().getData();
        try {
            readFileFromUri(DivActivity.this, uri);
        } catch (IOException e) {
            e.printStackTrace();
        }
        if (bufferlen > 0) {
            int numOfBlocks = bufferlen / 250;
            int lengthLastBlock = bufferlen % 250;
            if (lengthLastBlock != 0) {
                numOfBlocks++;
            }
            Log.i("THMIC64", "size of file = " + bufferlen + ", num of blocks = " + numOfBlocks);
            showProgressDialog();
            int finalNumOfBlocks = numOfBlocks;
            new Thread(new Runnable() {
                @Override
                public void run() {
                    boolean first = true;
                    boolean last = false;
                    for (int i = 0; i < finalNumOfBlocks; i++) {
                        if (isCancelled) {
                            runOnUiThread(() -> Toast.makeText(DivActivity.this, "Transfer canceled", Toast.LENGTH_SHORT).show());
                            return;
                        }
                        int finalI = i;
                        int remainingBlocks = finalNumOfBlocks - i;
                        runOnUiThread(() -> updateProgress(finalI, finalNumOfBlocks));
                        if (i == (finalNumOfBlocks - 1)) {
                            last = true;
                        }
                        sendNextBlock(i, first, last, lengthLastBlock);
                        wait4Ack();
                        first = false;
                    }
                    // Transfer beendet
                    runOnUiThread(() -> {
                        dismissProgressDialog();
                        Toast.makeText(DivActivity.this, "transfer finished", Toast.LENGTH_SHORT).show();
                    });
                }
            }).start();
        }
    }

    @Override
    public void update() {
        ackFromServer = true;
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

        myApplication = (MyApplication) getApplication();
        settings = myApplication.getSettings();

        type4Notification = myApplication.getType4Notification();
        type4Notification.registerObserver(this);

        setContentView(R.layout.div);

        toggleRefreshframecolorSwitch = findViewById(R.id.toggleRefreshframecolor);
        toggleRefreshframecolorSwitch.setOnClickListener(v -> sendCmd(new byte[]{Config.SWITCHFRAMECOLORREFRESH, (byte) 0x00, (byte) 0x80}, false));

        toggleSendRawKeyCodes = findViewById(R.id.toggleSendRawKeyCodes);
        toggleSendRawKeyCodes.setOnClickListener(v -> sendCmd(new byte[]{Config.SENDRAWKEYS, (byte) 0x00, (byte) 0x80}, false));

        toggleDebug = findViewById(R.id.toggleDebug);
        toggleDebug.setOnClickListener(v -> sendCmd(new byte[]{Config.SWITCHDEBUG, (byte) 0x00, (byte) 0x80}, false));

        togglePerf = findViewById(R.id.togglePerf);
        togglePerf.setOnClickListener(v -> sendCmd(new byte[]{Config.SWITCHPERF, (byte) 0x00, (byte) 0x80}, false));

        toggleDetectReleaseKey = findViewById(R.id.toggleDetectReleaseKey);
        toggleDetectReleaseKey.setOnClickListener(v -> sendCmd(new byte[]{Config.SWITCHDETECTRELEASEKEY, (byte) 0x00, (byte) 0x80}, false));

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

        filePickerLauncher = registerForActivityResult(
                new ActivityResultContracts.StartActivityForResult(),
                result -> {
                    if (result.getResultCode() == Activity.RESULT_OK && result.getData() != null) {
                        BLEManager bleManager = myApplication.getBleManager();
                        if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
                            startTransfer(result);
                        } else {
                            Toast.makeText(DivActivity.this, "no BLE connection, please connect to emulator", Toast.LENGTH_SHORT).show();
                        }
                    }
                });

        final Button send = findViewById(R.id.send);
        send.setOnClickListener(view -> openFilePicker());

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
        resetButton.setOnClickListener(v -> sendCmd(new byte[]{Config.RESET, (byte) 0x00, (byte) 0x80}, false));

        final Button closeButton = findViewById(R.id.close);
        closeButton.setOnClickListener(v -> finish());

        updateSettings();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        type4Notification.removeObserver();
    }
}
