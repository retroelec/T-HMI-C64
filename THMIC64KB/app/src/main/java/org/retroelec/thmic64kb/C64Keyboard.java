package org.retroelec.thmic64kb;

import android.content.Context;
import android.graphics.Color;
import android.os.Handler;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;

import java.util.HashMap;
import java.util.Map;

public class C64Keyboard extends LinearLayout {

    private static final int KEYSELECTEDCOLOR = 0xff777777;
    private static final int KEYBGCOLOR = 0xff2b2121;
    private static final int KEYSELECTEDCOLORFKEYS = 0xff888888;
    private static final int KEYBGCOLORFKEYS = 0xff777777;
    private static final int KEYSELECTEDCOLORFRESETOFF = 0xff999999;
    private static final int KEYBGCOLORFRESETOFF = 0xff888888;

    private BLEUtils bleUtils;
    private Settings settings;

    private final Map<String, byte[]> map = new HashMap<>();
    private Button keyshiftleft;
    private Button keyshiftlock;
    private Button keyshiftright;
    private Button keyctrl;
    private ImageButton keycommodore;
    private Button key1;
    private Button key2;
    private Button key3;
    private Button key4;
    private Button key5;
    private Button key6;
    private Button key7;
    private Button key8;
    private Button key9;
    private Button key0;
    private Button keycolon;
    private Button keysemicolon;
    private Button keycomma;
    private Button keyperiod;
    private Button keyslash;
    private Button keyf1;
    private Button keyf3;
    private Button keyf5;
    private Button keyf7;
    private Button keyleftarrow;
    private Button keyplus;
    private Button keyminus;
    private Button keypound;
    private ImageButton keyhome;
    private ImageButton keydel;
    private Button keyq;
    private Button keyw;
    private Button keye;
    private Button keyr;
    private Button keyt;
    private Button keyy;
    private Button keyu;
    private Button keyi;
    private Button keyo;
    private Button keyp;
    private Button keyat;
    private Button keymul;
    private Button keyarrowup;
    private Button keyrestore;
    private ImageButton keyrunstop;
    private Button keya;
    private Button keys;
    private Button keyd;
    private Button keyf;
    private Button keyg;
    private Button keyh;
    private Button keyj;
    private Button keyk;
    private Button keyl;
    private Button keyequal;
    private Button keyreturn;
    private Button keyz;
    private Button keyx;
    private Button keyc;
    private Button keyv;
    private Button keyb;
    private Button keyn;
    private Button keym;
    private Button keycrsrdown;
    private Button keycrsrup;
    private Button keycrsrright;
    private Button keycrsrleft;
    private Button keyspace;

    void initKBHashMap() {
        map.put("RESTORE", new byte[]{Config.RESTORE, (byte) 0x00, (byte) 0x80});
        map.put("RESTORERUNSTOP", new byte[]{Config.RESTORE, (byte) 0x01, (byte) 0x80});
        map.put("del", new byte[]{(byte) 0xfe, (byte) 0xfe, (byte) 0x00});
        map.put("home", new byte[]{(byte) 0xbf, (byte) 0xf7, (byte) 0x00});
        map.put("ctrl", new byte[]{(byte) 0x7f, (byte) 0xfb, (byte) 0x00});
        map.put("runstop", new byte[]{(byte) 0x7f, (byte) 0x7f, (byte) 0x00});
        map.put("shiftleft", new byte[]{(byte) 0xfd, (byte) 0x7f, (byte) 0x00});
        map.put("return", new byte[]{(byte) 0xfe, (byte) 0xfd, (byte) 0x00});
        map.put("commodore", new byte[]{(byte) 0x7f, (byte) 0xdf, (byte) 0x00});
        map.put("shiftright", new byte[]{(byte) 0xbf, (byte) 0xef, (byte) 0x00});
        map.put("space", new byte[]{(byte) 0x7f, (byte) 0xef, (byte) 0x00});
        map.put("crsrright", new byte[]{(byte) 0xfe, (byte) 0xfb, (byte) 0x00});
        map.put("crsrleft", new byte[]{(byte) 0xfe, (byte) 0xfb, (byte) 0x01});
        map.put("crsrdown", new byte[]{(byte) 0xfe, (byte) 0x7f, (byte) 0x00});
        map.put("crsrup", new byte[]{(byte) 0xfe, (byte) 0x7f, (byte) 0x01});
        map.put("f1", new byte[]{(byte) 0xfe, (byte) 0xef, (byte) 0x00});
        map.put("f3", new byte[]{(byte) 0xfe, (byte) 0xdf, (byte) 0x00});
        map.put("f5", new byte[]{(byte) 0xfe, (byte) 0xbf, (byte) 0x00});
        map.put("f7", new byte[]{(byte) 0xfe, (byte) 0xf7, (byte) 0x00});
        map.put("3", new byte[]{(byte) 0xfd, (byte) 0xfe, (byte) 0x00});
        map.put("W", new byte[]{(byte) 0xfd, (byte) 0xfd, (byte) 0x00});
        map.put("A", new byte[]{(byte) 0xfd, (byte) 0xfb, (byte) 0x00});
        map.put("4", new byte[]{(byte) 0xfd, (byte) 0xf7, (byte) 0x00});
        map.put("Z", new byte[]{(byte) 0xfd, (byte) 0xef, (byte) 0x00});
        map.put("S", new byte[]{(byte) 0xfd, (byte) 0xdf, (byte) 0x00});
        map.put("E", new byte[]{(byte) 0xfd, (byte) 0xbf, (byte) 0x00});
        map.put("5", new byte[]{(byte) 0xfb, (byte) 0xfe, (byte) 0x00});
        map.put("R", new byte[]{(byte) 0xfb, (byte) 0xfd, (byte) 0x00});
        map.put("D", new byte[]{(byte) 0xfb, (byte) 0xfb, (byte) 0x00});
        map.put("6", new byte[]{(byte) 0xfb, (byte) 0xf7, (byte) 0x00});
        map.put("C", new byte[]{(byte) 0xfb, (byte) 0xef, (byte) 0x00});
        map.put("F", new byte[]{(byte) 0xfb, (byte) 0xdf, (byte) 0x00});
        map.put("T", new byte[]{(byte) 0xfb, (byte) 0xbf, (byte) 0x00});
        map.put("X", new byte[]{(byte) 0xfb, (byte) 0x7f, (byte) 0x00});
        map.put("7", new byte[]{(byte) 0xf7, (byte) 0xfe, (byte) 0x00});
        map.put("Y", new byte[]{(byte) 0xf7, (byte) 0xfd, (byte) 0x00});
        map.put("G", new byte[]{(byte) 0xf7, (byte) 0xfb, (byte) 0x00});
        map.put("8", new byte[]{(byte) 0xf7, (byte) 0xf7, (byte) 0x00});
        map.put("B", new byte[]{(byte) 0xf7, (byte) 0xef, (byte) 0x00});
        map.put("H", new byte[]{(byte) 0xf7, (byte) 0xdf, (byte) 0x00});
        map.put("U", new byte[]{(byte) 0xf7, (byte) 0xbf, (byte) 0x00});
        map.put("V", new byte[]{(byte) 0xf7, (byte) 0x7f, (byte) 0x00});
        map.put("9", new byte[]{(byte) 0xef, (byte) 0xfe, (byte) 0x00});
        map.put("I", new byte[]{(byte) 0xef, (byte) 0xfd, (byte) 0x00});
        map.put("J", new byte[]{(byte) 0xef, (byte) 0xfb, (byte) 0x00});
        map.put("0", new byte[]{(byte) 0xef, (byte) 0xf7, (byte) 0x00});
        map.put("M", new byte[]{(byte) 0xef, (byte) 0xef, (byte) 0x00});
        map.put("K", new byte[]{(byte) 0xef, (byte) 0xdf, (byte) 0x00});
        map.put("O", new byte[]{(byte) 0xef, (byte) 0xbf, (byte) 0x00});
        map.put("N", new byte[]{(byte) 0xef, (byte) 0x7f, (byte) 0x00});
        map.put("+", new byte[]{(byte) 0xdf, (byte) 0xfe, (byte) 0x00});
        map.put("P", new byte[]{(byte) 0xdf, (byte) 0xfd, (byte) 0x00});
        map.put("L", new byte[]{(byte) 0xdf, (byte) 0xfb, (byte) 0x00});
        map.put("-", new byte[]{(byte) 0xdf, (byte) 0xf7, (byte) 0x00});
        map.put(".", new byte[]{(byte) 0xdf, (byte) 0xef, (byte) 0x00});
        map.put(":", new byte[]{(byte) 0xdf, (byte) 0xdf, (byte) 0x00});
        map.put("@", new byte[]{(byte) 0xdf, (byte) 0xbf, (byte) 0x00});
        map.put(",", new byte[]{(byte) 0xdf, (byte) 0x7f, (byte) 0x00});
        map.put("£", new byte[]{(byte) 0xbf, (byte) 0xfe, (byte) 0x00});
        map.put("*", new byte[]{(byte) 0xbf, (byte) 0xfd, (byte) 0x00});
        map.put(";", new byte[]{(byte) 0xbf, (byte) 0xfb, (byte) 0x00});
        map.put("=", new byte[]{(byte) 0xbf, (byte) 0xdf, (byte) 0x00});
        map.put("uparrow", new byte[]{(byte) 0xbf, (byte) 0xbf, (byte) 0x00});
        map.put("/", new byte[]{(byte) 0xbf, (byte) 0x7f, (byte) 0x00});
        map.put("1", new byte[]{(byte) 0x7f, (byte) 0xfe, (byte) 0x00});
        map.put("leftarrow", new byte[]{(byte) 0x7f, (byte) 0xfd, (byte) 0x00});
        map.put("2", new byte[]{(byte) 0x7f, (byte) 0xf7, (byte) 0x00});
        map.put("Q", new byte[]{(byte) 0x7f, (byte) 0xbf, (byte) 0x00});
    }

    public C64Keyboard(Context context) {
        this(context, null, 0);
    }

    public C64Keyboard(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public C64Keyboard(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initKBHashMap();
        init(context);
    }

    enum ShiftKey {LEFT, RIGHT, LOCK}

    private void handleShiftKeys(boolean isShifted, ShiftKey shiftKey) {
        if (isShifted) {
            switch (shiftKey) {
                case LEFT:
                    keyshiftleft.setBackgroundColor(KEYSELECTEDCOLOR);
                    break;
                case RIGHT:
                    keyshiftright.setBackgroundColor(KEYSELECTEDCOLOR);
                    break;
                case LOCK:
                    keyshiftlock.setBackgroundColor(KEYSELECTEDCOLOR);
                    break;
            }
            key1.setText("!");
            key2.setText("\"");
            key3.setText("#");
            key4.setText("$");
            key5.setText("%");
            key6.setText("&");
            key7.setText("'");
            key8.setText("(");
            key9.setText(")");
            keycolon.setText("[");
            keysemicolon.setText("]");
            keycomma.setText("<");
            keyperiod.setText(">");
            keyslash.setText("?");
        } else {
            switch (shiftKey) {
                case LEFT:
                    keyshiftleft.setBackgroundColor(KEYBGCOLOR);
                    break;
                case RIGHT:
                    keyshiftright.setBackgroundColor(KEYBGCOLOR);
                    break;
                case LOCK:
                    keyshiftlock.setBackgroundColor(KEYBGCOLOR);
                    break;
            }
            key1.setText("1");
            key2.setText("2");
            key3.setText("3");
            key4.setText("4");
            key5.setText("5");
            key6.setText("6");
            key7.setText("7");
            key8.setText("8");
            key9.setText("9");
            keycolon.setText(":");
            keysemicolon.setText(";");
            keycomma.setText(",");
            keyperiod.setText(".");
            keyslash.setText("/");
        }
    }

    private void handleCtrlKey(boolean isPressed) {
        if (isPressed) {
            keyctrl.setBackgroundColor(KEYSELECTEDCOLOR);
            key1.setTextColor(Color.parseColor("#000000"));
            key2.setTextColor(Color.parseColor("#ffffff"));
            key3.setTextColor(Color.parseColor("#880000"));
            key4.setTextColor(Color.parseColor("#aaffee"));
            key5.setTextColor(Color.parseColor("#cc44cc"));
            key6.setTextColor(Color.parseColor("#00cc55"));
            key7.setTextColor(Color.parseColor("#0000aa"));
            key8.setTextColor(Color.parseColor("#eeee77"));
            key9.setBackgroundColor(Color.GRAY);
            key0.setBackgroundColor(Color.parseColor("#333333"));
        } else {
            keyctrl.setBackgroundColor(KEYBGCOLOR);
            key1.setTextColor(Color.WHITE);
            key2.setTextColor(Color.WHITE);
            key3.setTextColor(Color.WHITE);
            key4.setTextColor(Color.WHITE);
            key5.setTextColor(Color.WHITE);
            key6.setTextColor(Color.WHITE);
            key7.setTextColor(Color.WHITE);
            key8.setTextColor(Color.WHITE);
            key9.setBackgroundColor(Color.parseColor("#2b2121"));
            key0.setBackgroundColor(Color.parseColor("#2b2121"));
        }
    }

    private void handleCommodoreKey(boolean isPressed) {
        if (isPressed) {
            keycommodore.setBackgroundColor(KEYSELECTEDCOLOR);
            key1.setTextColor(Color.parseColor("#dd8855"));
            key2.setTextColor(Color.parseColor("#664400"));
            key3.setTextColor(Color.parseColor("#ff7777"));
            key4.setTextColor(Color.parseColor("#333333"));
            key5.setTextColor(Color.parseColor("#777777"));
            key6.setTextColor(Color.parseColor("#aaff66"));
            key7.setTextColor(Color.parseColor("#0088ff"));
            key8.setTextColor(Color.parseColor("#bbbbbb"));
        } else {
            keycommodore.setBackgroundColor(KEYBGCOLOR);
            key1.setTextColor(Color.WHITE);
            key2.setTextColor(Color.WHITE);
            key3.setTextColor(Color.WHITE);
            key4.setTextColor(Color.WHITE);
            key5.setTextColor(Color.WHITE);
            key6.setTextColor(Color.WHITE);
            key7.setTextColor(Color.WHITE);
            key8.setTextColor(Color.WHITE);
        }
    }

    private void sendKey(Context context, byte[] data) {
        Vibrator vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        if (vibrator != null && vibrator.hasVibrator()) {
            vibrator.vibrate(VibrationEffect.createOneShot(Config.DURATION_VIBRATION_EFFECT, VibrationEffect.DEFAULT_AMPLITUDE));
        }
        boolean isShifted = keyshiftleft.isSelected() || keyshiftright.isSelected();
        boolean isCtrl = keyctrl.isSelected();
        boolean isCommodore = keycommodore.isSelected();
        byte[] datatosend = data;
        if (isShifted) {
            keyshiftleft.setSelected(false);
            keyshiftleft.setBackgroundColor(KEYBGCOLOR);
            keyshiftright.setSelected(false);
            keyshiftright.setBackgroundColor(KEYBGCOLOR);
            handleShiftKeys(false, ShiftKey.LEFT);
            handleShiftKeys(false, ShiftKey.RIGHT);
        } else if (isCtrl) {
            keyctrl.setSelected(false);
            keyctrl.setBackgroundColor(KEYBGCOLOR);
            handleCtrlKey(false);
        } else if (isCommodore) {
            keycommodore.setSelected(false);
            keycommodore.setBackgroundColor(KEYBGCOLOR);
            handleCommodoreKey(false);
        }
        if (isShifted || keyshiftlock.isSelected()) {
            datatosend = new byte[]{data[0], data[1], 1};
        } else if (isCtrl) {
            datatosend = new byte[]{data[0], data[1], 2};
        } else if (isCommodore) {
            datatosend = new byte[]{data[0], data[1], 4};
        }
        bleUtils.send(datatosend, false);
    }

    @FunctionalInterface
    interface ActionDownHandler {
        boolean handleActionDown(Context context, String key);
    }


    private View.OnTouchListener createButtonTouchListener(String key, ActionDownHandler actionDownHandler, boolean visualEffect) {
        return bleUtils.createButtonTouchListener(key, actionDownHandler, visualEffect, KEYSELECTEDCOLOR, KEYBGCOLOR);
    }

    private View.OnTouchListener createTouchListenerFKeys(String key, ActionDownHandler actionDownHandler, boolean visualEffect) {
        return bleUtils.createButtonTouchListener(key, actionDownHandler, visualEffect, KEYSELECTEDCOLORFKEYS, KEYBGCOLORFKEYS);
    }

    private final ActionDownHandler standardActionDown = (context, key) -> {
        sendKey(context, map.get(key));
        return true;
    };

    private final ActionDownHandler shiftLeftActionDown = (context, key) -> {
        if (settings.isSendRawKeyCodes()) {
            keyshiftleft.setBackgroundColor(KEYSELECTEDCOLOR);
            new Handler().postDelayed(() -> keyshiftleft.setBackgroundColor(KEYBGCOLOR), Config.DURATION_VISUAL_EFFECT);
            sendKey(context, map.get(key));
        } else if (keycommodore.isSelected() && (!keyshiftleft.isSelected())) {
            sendKey(context, new byte[]{(byte) 0xfd, (byte) 0x7f, (byte) 0x05});
        } else {
            if (keyshiftlock.isSelected()) {
                return true;
            }
            keyshiftleft.setSelected(!keyshiftleft.isSelected());
            Log.d("THMIC64", "shift button clicked, selected: " + keyshiftleft.isSelected());
            handleShiftKeys(keyshiftleft.isSelected(), ShiftKey.LEFT);
        }
        return true;
    };

    private final ActionDownHandler shiftRightActionDown = (context, key) -> {
        if (settings.isSendRawKeyCodes()) {
            keyshiftright.setBackgroundColor(KEYSELECTEDCOLOR);
            new Handler().postDelayed(() -> keyshiftright.setBackgroundColor(KEYBGCOLOR), Config.DURATION_VISUAL_EFFECT);
            sendKey(context, map.get(key));
        } else if (keycommodore.isSelected() && (!keyshiftright.isSelected())) {
            sendKey(context, new byte[]{(byte) 0xbf, (byte) 0xef, (byte) 0x05});
        } else {
            if (keyshiftlock.isSelected()) {
                return true;
            }
            keyshiftright.setSelected(!keyshiftright.isSelected());
            Log.d("THMIC64", "shift button clicked, selected: " + keyshiftright.isSelected());
            handleShiftKeys(keyshiftright.isSelected(), ShiftKey.RIGHT);
        }
        return true;
    };

    private final ActionDownHandler ctrlActionDown = (context, key) -> {
        if (settings.isSendRawKeyCodes()) {
            keyctrl.setBackgroundColor(KEYSELECTEDCOLOR);
            new Handler().postDelayed(() -> keyctrl.setBackgroundColor(KEYBGCOLOR), Config.DURATION_VISUAL_EFFECT);
            sendKey(context, map.get(key));
        } else {
            keyctrl.setSelected(!keyctrl.isSelected());
            Log.d("THMIC64", "ctrl button clicked, selected: " + keyctrl.isSelected());
            handleCtrlKey(keyctrl.isSelected());
        }
        return true;
    };

    private final ActionDownHandler restoreActionDownHandler = (context, key) -> {
        // combination "RUN/STOP" + "RESTORE" is replaced by "COMMODRE" + "RESTORE"
        if (keycommodore.isSelected()) {
            keycommodore.setSelected(false);
            handleCommodoreKey(false);
            sendKey(context, map.get("RESTORERUNSTOP"));
        } else {
            sendKey(context, map.get(key));
        }
        return true;
    };

    private final ActionDownHandler shiftlockActionDown = (context, key) -> {
        keyshiftlock.setSelected(!keyshiftlock.isSelected());
        handleShiftKeys(keyshiftlock.isSelected(), ShiftKey.LOCK);
        return true;
    };

    private final ActionDownHandler commodoreActionDown = (context, key) -> {
        if (settings.isSendRawKeyCodes()) {
            keycommodore.setBackgroundColor(KEYSELECTEDCOLOR);
            new Handler().postDelayed(() -> keycommodore.setBackgroundColor(KEYBGCOLOR), Config.DURATION_VISUAL_EFFECT);
            sendKey(context, map.get(key));
        } else if ((keyshiftleft.isSelected() || keyshiftright.isSelected()) && (!keycommodore.isSelected())) {
            sendKey(context, new byte[]{(byte) 0x7f, (byte) 0xdf, (byte) 0x05});
        } else {
            keycommodore.setSelected(!keycommodore.isSelected());
            Log.d("THMIC64", "commodore button clicked, selected: " + keycommodore.isSelected());
            handleCommodoreKey(keycommodore.isSelected());
        }
        return true;
    };

    private void init(Context context) {
        MyApplication myApplication = (MyApplication) context.getApplicationContext();
        settings = myApplication.getSettings();
        bleUtils = new BLEUtils(context);
        LayoutInflater.from(context).inflate(R.layout.keyboard, this, true);
        keyshiftleft = findViewById(R.id.keyshiftleft);
        keyf1 = findViewById(R.id.keyf1);
        keyf3 = findViewById(R.id.keyf3);
        keyf5 = findViewById(R.id.keyf5);
        keyf7 = findViewById(R.id.keyf7);
        keyleftarrow = findViewById(R.id.keyleftarrow);
        key1 = findViewById(R.id.key1);
        key2 = findViewById(R.id.key2);
        key3 = findViewById(R.id.key3);
        key4 = findViewById(R.id.key4);
        key5 = findViewById(R.id.key5);
        key6 = findViewById(R.id.key6);
        key7 = findViewById(R.id.key7);
        key8 = findViewById(R.id.key8);
        key9 = findViewById(R.id.key9);
        key0 = findViewById(R.id.key0);
        keyplus = findViewById(R.id.keyplus);
        keyminus = findViewById(R.id.keyminus);
        keypound = findViewById(R.id.keypound);
        keyhome = findViewById(R.id.keyhome);
        keyhome.setBackgroundColor(KEYBGCOLOR);
        keydel = findViewById(R.id.keydel);
        keydel.setBackgroundColor(KEYBGCOLOR);
        keyctrl = findViewById(R.id.keyctrl);
        keyq = findViewById(R.id.keyq);
        keyw = findViewById(R.id.keyw);
        keye = findViewById(R.id.keye);
        keyr = findViewById(R.id.keyr);
        keyt = findViewById(R.id.keyt);
        keyy = findViewById(R.id.keyy);
        keyu = findViewById(R.id.keyu);
        keyi = findViewById(R.id.keyi);
        keyo = findViewById(R.id.keyo);
        keyp = findViewById(R.id.keyp);
        keyat = findViewById(R.id.keyat);
        keymul = findViewById(R.id.keymul);
        keyarrowup = findViewById(R.id.keyarrowup);
        keyrestore = findViewById(R.id.keyrestore);
        keyrunstop = findViewById(R.id.keyrunstop);
        keyrunstop.setBackgroundColor(KEYBGCOLOR);
        keyshiftlock = findViewById(R.id.keyshiftlock);
        keya = findViewById(R.id.keya);
        keys = findViewById(R.id.keys);
        keyd = findViewById(R.id.keyd);
        keyf = findViewById(R.id.keyf);
        keyg = findViewById(R.id.keyg);
        keyh = findViewById(R.id.keyh);
        keyj = findViewById(R.id.keyj);
        keyk = findViewById(R.id.keyk);
        keyl = findViewById(R.id.keyl);
        keycolon = findViewById(R.id.keycolon);
        keysemicolon = findViewById(R.id.keysemicolon);
        keyequal = findViewById(R.id.keyequal);
        keyreturn = findViewById(R.id.keyreturn);
        keycommodore = findViewById(R.id.keycommodore);
        keycommodore.setBackgroundColor(KEYBGCOLOR);
        keyz = findViewById(R.id.keyz);
        keyx = findViewById(R.id.keyx);
        keyc = findViewById(R.id.keyc);
        keyv = findViewById(R.id.keyv);
        keyb = findViewById(R.id.keyb);
        keyn = findViewById(R.id.keyn);
        keym = findViewById(R.id.keym);
        keycomma = findViewById(R.id.keycomma);
        keyperiod = findViewById(R.id.keyperiod);
        keyslash = findViewById(R.id.keyslash);
        keyshiftright = findViewById(R.id.keyshiftright);
        keycrsrdown = findViewById(R.id.keycrsrdown);
        keycrsrup = findViewById(R.id.keycrsrup);
        keycrsrright = findViewById(R.id.keycrsrright);
        keycrsrleft = findViewById(R.id.keycrsrleft);
        keyspace = findViewById(R.id.keyspace);
        keyf1.setOnTouchListener(createTouchListenerFKeys("f1", standardActionDown, true));
        keyf3.setOnTouchListener(createTouchListenerFKeys("f3", standardActionDown, true));
        keyf5.setOnTouchListener(createTouchListenerFKeys("f5", standardActionDown, true));
        keyf7.setOnTouchListener(createTouchListenerFKeys("f7", standardActionDown, true));
        keyleftarrow.setOnTouchListener(createButtonTouchListener("leftarrow", standardActionDown, true));
        key1.setOnTouchListener(createButtonTouchListener("1", standardActionDown, true));
        key2.setOnTouchListener(createButtonTouchListener("2", standardActionDown, true));
        key3.setOnTouchListener(createButtonTouchListener("3", standardActionDown, true));
        key4.setOnTouchListener(createButtonTouchListener("4", standardActionDown, true));
        key5.setOnTouchListener(createButtonTouchListener("5", standardActionDown, true));
        key6.setOnTouchListener(createButtonTouchListener("6", standardActionDown, true));
        key7.setOnTouchListener(createButtonTouchListener("7", standardActionDown, true));
        key8.setOnTouchListener(createButtonTouchListener("8", standardActionDown, true));
        key9.setOnTouchListener(createButtonTouchListener("9", standardActionDown, true));
        key0.setOnTouchListener(createButtonTouchListener("0", standardActionDown, true));
        keyplus.setOnTouchListener(createButtonTouchListener("+", standardActionDown, true));
        keyminus.setOnTouchListener(createButtonTouchListener("-", standardActionDown, true));
        keypound.setOnTouchListener(createButtonTouchListener("£", standardActionDown, true));
        keyhome.setOnTouchListener(createButtonTouchListener("home", standardActionDown, true));
        keydel.setOnTouchListener(createButtonTouchListener("del", standardActionDown, true));
        keyctrl.setOnTouchListener(createButtonTouchListener("ctrl", ctrlActionDown, false));
        keyq.setOnTouchListener(createButtonTouchListener("Q", standardActionDown, true));
        keyw.setOnTouchListener(createButtonTouchListener("W", standardActionDown, true));
        keye.setOnTouchListener(createButtonTouchListener("E", standardActionDown, true));
        keyr.setOnTouchListener(createButtonTouchListener("R", standardActionDown, true));
        keyt.setOnTouchListener(createButtonTouchListener("T", standardActionDown, true));
        keyy.setOnTouchListener(createButtonTouchListener("Y", standardActionDown, true));
        keyu.setOnTouchListener(createButtonTouchListener("U", standardActionDown, true));
        keyi.setOnTouchListener(createButtonTouchListener("I", standardActionDown, true));
        keyo.setOnTouchListener(createButtonTouchListener("O", standardActionDown, true));
        keyp.setOnTouchListener(createButtonTouchListener("P", standardActionDown, true));
        keyat.setOnTouchListener(createButtonTouchListener("@", standardActionDown, true));
        keymul.setOnTouchListener(createButtonTouchListener("*", standardActionDown, true));
        keyarrowup.setOnTouchListener(createButtonTouchListener("uparrow", standardActionDown, true));
        keyrestore.setOnTouchListener(createButtonTouchListener("RESTORE", restoreActionDownHandler, true));
        keyrunstop.setOnTouchListener(createButtonTouchListener("runstop", standardActionDown, true));
        keyshiftlock.setOnTouchListener(createButtonTouchListener("shiftlock", shiftlockActionDown, false));
        keycommodore.setOnTouchListener(createButtonTouchListener("commodore", commodoreActionDown, false));
        keya.setOnTouchListener(createButtonTouchListener("A", standardActionDown, true));
        keys.setOnTouchListener(createButtonTouchListener("S", standardActionDown, true));
        keyd.setOnTouchListener(createButtonTouchListener("D", standardActionDown, true));
        keyf.setOnTouchListener(createButtonTouchListener("F", standardActionDown, true));
        keyg.setOnTouchListener(createButtonTouchListener("G", standardActionDown, true));
        keyh.setOnTouchListener(createButtonTouchListener("H", standardActionDown, true));
        keyj.setOnTouchListener(createButtonTouchListener("J", standardActionDown, true));
        keyk.setOnTouchListener(createButtonTouchListener("K", standardActionDown, true));
        keyl.setOnTouchListener(createButtonTouchListener("L", standardActionDown, true));
        keycolon.setOnTouchListener(createButtonTouchListener(":", standardActionDown, true));
        keysemicolon.setOnTouchListener(createButtonTouchListener(";", standardActionDown, true));
        keyequal.setOnTouchListener(createButtonTouchListener("=", standardActionDown, true));
        keyreturn.setOnTouchListener(createButtonTouchListener("return", standardActionDown, true));
        keyz.setOnTouchListener(createButtonTouchListener("Z", standardActionDown, true));
        keyx.setOnTouchListener(createButtonTouchListener("X", standardActionDown, true));
        keyc.setOnTouchListener(createButtonTouchListener("C", standardActionDown, true));
        keyv.setOnTouchListener(createButtonTouchListener("V", standardActionDown, true));
        keyb.setOnTouchListener(createButtonTouchListener("B", standardActionDown, true));
        keyn.setOnTouchListener(createButtonTouchListener("N", standardActionDown, true));
        keym.setOnTouchListener(createButtonTouchListener("M", standardActionDown, true));
        keycomma.setOnTouchListener(createButtonTouchListener(",", standardActionDown, true));
        keyperiod.setOnTouchListener(createButtonTouchListener(".", standardActionDown, true));
        keyslash.setOnTouchListener(createButtonTouchListener("/", standardActionDown, true));
        keyshiftleft.setOnTouchListener(createButtonTouchListener("shiftleft", shiftLeftActionDown, false));
        keyshiftright.setOnTouchListener(createButtonTouchListener("shiftright", shiftRightActionDown, false));
        keycrsrdown.setOnTouchListener(createButtonTouchListener("crsrdown", standardActionDown, true));
        keycrsrup.setOnTouchListener(createButtonTouchListener("crsrup", standardActionDown, true));
        keycrsrright.setOnTouchListener(createButtonTouchListener("crsrright", standardActionDown, true));
        keycrsrleft.setOnTouchListener(createButtonTouchListener("crsrleft", standardActionDown, true));
        keyspace.setOnTouchListener(createButtonTouchListener("space", standardActionDown, true));

        Button powerOff = findViewById(R.id.powerOff);
        powerOff.setOnTouchListener(bleUtils.createButtonTouchListener(powerOff, new byte[]{Config.POWEROFF, (byte) 0x00, (byte) 0x80}, KEYSELECTEDCOLORFRESETOFF, KEYBGCOLORFRESETOFF, true));

        Button reset = findViewById(R.id.reset);
        reset.setOnTouchListener(bleUtils.createButtonTouchListener(reset, new byte[]{Config.RESET, (byte) 0x00, (byte) 0x80}, KEYSELECTEDCOLORFRESETOFF, KEYBGCOLORFRESETOFF, true));
    }
}
