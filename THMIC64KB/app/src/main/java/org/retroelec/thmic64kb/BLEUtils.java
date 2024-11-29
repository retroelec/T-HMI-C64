package org.retroelec.thmic64kb;

import android.content.Context;
import android.graphics.Color;
import android.os.Handler;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.view.MotionEvent;
import android.view.View;

public class BLEUtils {
    private final Context context;
    private final MyApplication myApplication;
    private final Settings settings;

    private long startTime = 0;
    private final Handler handler = new Handler();

    public BLEUtils(Context context) {
        this.context = context;
        this.myApplication = (MyApplication) context.getApplicationContext();
        this.settings = myApplication.getSettings();
    }

    public void send(byte[] data, boolean blocking) {
        BLEManager bleManager = myApplication.getBleManager();
        if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
            bleManager.sendData(data, blocking);
        }
    }

    public void send(byte[] data) {
        send(data, false);
    }

    public void send(byte data) {
        byte[] datatosend = new byte[]{data};
        send(datatosend);
    }

    public View.OnTouchListener createButtonTouchListener(View button, byte[] data, int selcolor, int bgcolor, boolean vibration) {
        return (arg0, event) -> {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                if (vibration) {
                    Vibrator vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
                    if (vibrator != null && vibrator.hasVibrator()) {
                        vibrator.vibrate(VibrationEffect.createOneShot(Config.DURATION_VIBRATION_EFFECT, VibrationEffect.DEFAULT_AMPLITUDE));
                    }
                }
                button.setBackgroundColor(selcolor);
                send(data);
                startTime = System.currentTimeMillis();
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                button.setBackgroundColor(bgcolor);
                long endTime = System.currentTimeMillis();
                long duration = endTime - startTime;
                long delay = settings.getMinKeyPressedDuration() - duration;
                if (delay > 0) {
                    handler.postDelayed(() -> send(Config.KEYRELEASED), delay);
                } else {
                    send(Config.KEYRELEASED);
                }
            }
            return true;
        };
    }

    public View.OnTouchListener createButtonTouchListener(View button, byte[] data) {
        return createButtonTouchListener(button, data, Color.GREEN, Color.TRANSPARENT, false);
    }

    public View.OnTouchListener createButtonTouchListener(View button, byte constActivated, byte constDeactivated) {
        return (arg0, event) -> {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                button.setBackgroundColor(Color.GREEN);
                send(constActivated);
                startTime = System.currentTimeMillis();
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                button.setBackgroundColor(Color.TRANSPARENT);
                long endTime = System.currentTimeMillis();
                long duration = endTime - startTime;
                long delay = settings.getMinKeyPressedDuration() - duration;
                if (delay > 0) {
                    handler.postDelayed(() -> send(constDeactivated), delay);
                } else {
                    send(constDeactivated);
                }
            }
            return true;
        };
    }

    public View.OnTouchListener createButtonTouchListener(String key, C64Keyboard.ActionDownHandler actionDownHandler, boolean visualEffect, int selcolor, int bgcolor) {
        return (view, event) -> {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                if (visualEffect) {
                    view.setBackgroundColor(selcolor);
                }
                startTime = System.currentTimeMillis();
                return actionDownHandler.handleActionDown(context, key);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                if (visualEffect) {
                    view.setBackgroundColor(bgcolor);
                }
                if (settings.isDetectReleaseKey()) {
                    long endTime = System.currentTimeMillis();
                    long duration = endTime - startTime;
                    long delay = settings.getMinKeyPressedDuration() - duration;
                    if (delay > 0) {
                        handler.postDelayed(() -> send(Config.KEYRELEASED), delay);
                    } else {
                        send(Config.KEYRELEASED);
                    }
                }
                return true;
            }
            return false;
        };
    }
}
