package org.retroelec.thmic64kb;

import android.content.Context;
import android.graphics.Color;
import android.os.Handler;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;

import java.util.HashMap;
import java.util.Map;

public class C64Keyboard extends LinearLayout {

    private static final int KEYBGCOLOR = 0xff2b2121;
    private static final int KEYSELECTEDCOLOR = 0xff777777;

    private MyApplication myApplication;
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

    void initKBHashMap() {
        map.put("LOAD", new byte[]{(byte) Config.LOAD, (byte) 0x00, (byte) 0x80});
        map.put("RESTORE", new byte[]{(byte) Config.RESTORE, (byte) 0x00, (byte) 0x80});
        map.put("RESTORERUNSTOP", new byte[]{(byte) Config.RESTORE, (byte) 0x01, (byte) 0x80});
        map.put("del", new byte[]{(byte) 0xfe, (byte) 0xfe, (byte) 0x00});
        map.put("home", new byte[]{(byte) 0xbf, (byte) 0xf7, (byte) 0x00});
        map.put("ctrl", new byte[]{(byte) 0x7f, (byte) 0xdf, (byte) 0x00});
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
            vibrator.vibrate(VibrationEffect.createOneShot(75, VibrationEffect.DEFAULT_AMPLITUDE));
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
        BLEManager bleManager = myApplication.getBleManager();
        if ((bleManager != null) && (bleManager.getCharacteristic() != null)) {
            bleManager.sendData(datatosend);
        }
    }

    private void init(Context context) {
        myApplication = (MyApplication) context.getApplicationContext();
        settings = myApplication.getSettings();

        Button keyf1;
        Button keyf3;
        Button keyf5;
        Button keyf7;
        Button keyload;
        Button keyleftarrow;
        Button keyplus;
        Button keyminus;
        Button keypound;
        ImageButton keyhome;
        ImageButton keydel;
        Button keyq;
        Button keyw;
        Button keye;
        Button keyr;
        Button keyt;
        Button keyy;
        Button keyu;
        Button keyi;
        Button keyo;
        Button keyp;
        Button keyat;
        Button keymul;
        Button keyarrowup;
        Button keyrestore;
        ImageButton keyrunstop;
        Button keya;
        Button keys;
        Button keyd;
        Button keyf;
        Button keyg;
        Button keyh;
        Button keyj;
        Button keyk;
        Button keyl;
        Button keyequal;
        Button keyreturn;
        Button keyz;
        Button keyx;
        Button keyc;
        Button keyv;
        Button keyb;
        Button keyn;
        Button keym;
        Button keycrsrdown;
        Button keycrsrup;
        Button keycrsrright;
        Button keycrsrleft;
        Button keyspace;
        LayoutInflater.from(context).inflate(R.layout.keyboard, this, true);
        keyshiftleft = findViewById(R.id.keyshiftleft);
        keyf1 = findViewById(R.id.keyf1);
        keyf3 = findViewById(R.id.keyf3);
        keyf5 = findViewById(R.id.keyf5);
        keyf7 = findViewById(R.id.keyf7);
        keyload = findViewById(R.id.keyload);
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

        keyf1.setOnClickListener(view -> {
            String key = "f1";
            sendKey(context, map.get(key));
        });
        keyf3.setOnClickListener(view -> {
            String key = "f3";
            sendKey(context, map.get(key));
        });
        keyf5.setOnClickListener(view -> {
            String key = "f5";
            sendKey(context, map.get(key));
        });
        keyf7.setOnClickListener(view -> {
            String key = "f7";
            sendKey(context, map.get(key));
        });
        keyload.setOnClickListener(view -> {
            String key = "LOAD";
            sendKey(context, map.get(key));
        });
        keyleftarrow.setOnClickListener(view -> {
            String key = "leftarrow";
            sendKey(context, map.get(key));
        });
        key1.setOnClickListener(view -> {
            String key = "1";
            sendKey(context, map.get(key));
        });
        key2.setOnClickListener(view -> {
            String key = "2";
            sendKey(context, map.get(key));
        });
        key3.setOnClickListener(view -> {
            String key = "3";
            sendKey(context, map.get(key));
        });
        key4.setOnClickListener(view -> {
            String key = "4";
            sendKey(context, map.get(key));
        });
        key5.setOnClickListener(view -> {
            String key = "5";
            sendKey(context, map.get(key));
        });
        key6.setOnClickListener(view -> {
            String key = "6";
            sendKey(context, map.get(key));
        });
        key7.setOnClickListener(view -> {
            String key = "7";
            sendKey(context, map.get(key));
        });
        key8.setOnClickListener(view -> {
            String key = "8";
            sendKey(context, map.get(key));
        });
        key9.setOnClickListener(view -> {
            String key = "9";
            sendKey(context, map.get(key));
        });
        key0.setOnClickListener(view -> {
            String key = "0";
            sendKey(context, map.get(key));
        });
        keyplus.setOnClickListener(view -> {
            String key = "+";
            sendKey(context, map.get(key));
        });
        keyminus.setOnClickListener(view -> {
            String key = "-";
            sendKey(context, map.get(key));
        });
        keypound.setOnClickListener(view -> {
            String key = "£";
            sendKey(context, map.get(key));
        });
        keyhome.setOnClickListener(view -> {
            keyhome.setBackgroundColor(KEYSELECTEDCOLOR);
            new Handler().postDelayed(() -> keyhome.setBackgroundColor(KEYBGCOLOR), 200);
            String key = "home";
            sendKey(context, map.get(key));
        });
        keydel.setOnClickListener(view -> {
            keydel.setBackgroundColor(KEYSELECTEDCOLOR);
            new Handler().postDelayed(() -> keydel.setBackgroundColor(KEYBGCOLOR), 200);
            String key = "del";
            sendKey(context, map.get(key));
        });
        keyctrl.setOnClickListener(view -> {
            if (settings.isSendRawKeyCodes()) {
                String key = "ctrl";
                sendKey(context, map.get(key));
            } else {
                keyctrl.setSelected(!keyctrl.isSelected());
                Log.d("THMIC64", "ctrl button clicked, selected: " + keyctrl.isSelected());
                handleCtrlKey(keyctrl.isSelected());
            }
        });
        keyq.setOnClickListener(view -> {
            String key = "Q";
            sendKey(context, map.get(key));
        });
        keyw.setOnClickListener(view -> {
            String key = "W";
            sendKey(context, map.get(key));
        });
        keye.setOnClickListener(view -> {
            String key = "E";
            sendKey(context, map.get(key));
        });
        keyr.setOnClickListener(view -> {
            String key = "R";
            sendKey(context, map.get(key));
        });
        keyt.setOnClickListener(view -> {
            String key = "T";
            sendKey(context, map.get(key));
        });
        keyy.setOnClickListener(view -> {
            String key = "Y";
            sendKey(context, map.get(key));
        });
        keyu.setOnClickListener(view -> {
            String key = "U";
            sendKey(context, map.get(key));
        });
        keyi.setOnClickListener(view -> {
            String key = "I";
            sendKey(context, map.get(key));
        });
        keyo.setOnClickListener(view -> {
            String key = "O";
            sendKey(context, map.get(key));
        });
        keyp.setOnClickListener(view -> {
            String key = "P";
            sendKey(context, map.get(key));
        });
        keyat.setOnClickListener(view -> {
            String key = "@";
            sendKey(context, map.get(key));
        });
        keymul.setOnClickListener(view -> {
            String key = "*";
            sendKey(context, map.get(key));
        });
        keyarrowup.setOnClickListener(view -> {
            String key = "uparrow";
            sendKey(context, map.get(key));
        });
        keyrestore.setOnClickListener(view -> {
            // combination "RUN/STOP" + "RESTORE" is replaced by "COMMODRE" + "RESTORE"
            if (keycommodore.isSelected()) {
                keycommodore.setSelected(false);
                handleCommodoreKey(false);
                String key = "RESTORERUNSTOP";
                sendKey(context, map.get(key));
            } else {
                String key = "RESTORE";
                sendKey(context, map.get(key));
            }
        });
        keyrunstop.setOnClickListener(view -> {
            keyrunstop.setBackgroundColor(KEYSELECTEDCOLOR);
            new Handler().postDelayed(() -> keyrunstop.setBackgroundColor(KEYBGCOLOR), 200);
            String key = "runstop";
            sendKey(context, map.get(key));
        });
        keyshiftlock.setOnClickListener(view -> {
            keyshiftlock.setSelected(!keyshiftlock.isSelected());
            handleShiftKeys(keyshiftlock.isSelected(), ShiftKey.LOCK);
        });
        keya.setOnClickListener(view -> {
            String key = "A";
            sendKey(context, map.get(key));
        });
        keys.setOnClickListener(view -> {
            String key = "S";
            sendKey(context, map.get(key));
        });
        keyd.setOnClickListener(view -> {
            String key = "D";
            sendKey(context, map.get(key));
        });
        keyf.setOnClickListener(view -> {
            String key = "F";
            sendKey(context, map.get(key));
        });
        keyg.setOnClickListener(view -> {
            String key = "G";
            sendKey(context, map.get(key));
        });
        keyh.setOnClickListener(view -> {
            String key = "H";
            sendKey(context, map.get(key));
        });
        keyj.setOnClickListener(view -> {
            String key = "J";
            sendKey(context, map.get(key));
        });
        keyk.setOnClickListener(view -> {
            String key = "K";
            sendKey(context, map.get(key));
        });
        keyl.setOnClickListener(view -> {
            String key = "L";
            sendKey(context, map.get(key));
        });
        keycolon.setOnClickListener(view -> {
            String key = ":";
            sendKey(context, map.get(key));
        });
        keysemicolon.setOnClickListener(view -> {
            String key = ";";
            sendKey(context, map.get(key));
        });
        keyequal.setOnClickListener(view -> {
            String key = "=";
            sendKey(context, map.get(key));
        });
        keyreturn.setOnClickListener(view -> {
            String key = "return";
            sendKey(context, map.get(key));
        });
        keycommodore.setOnClickListener(view -> {
            if (settings.isSendRawKeyCodes()) {
                keycommodore.setBackgroundColor(KEYSELECTEDCOLOR);
                new Handler().postDelayed(() -> keycommodore.setBackgroundColor(KEYBGCOLOR), 200);
                String key = "commodore";
                sendKey(context, map.get(key));
            } else {
                keycommodore.setSelected(!keycommodore.isSelected());
                Log.d("THMIC64", "commodore button clicked, selected: " + keycommodore.isSelected());
                handleCommodoreKey(keycommodore.isSelected());
            }
        });
        keyshiftleft.setOnClickListener(view -> {
            if (settings.isSendRawKeyCodes()) {
                String key = "shiftleft";
                sendKey(context, map.get(key));
            } else {
                if (keyshiftlock.isSelected()) {
                    return;
                }
                keyshiftleft.setSelected(!keyshiftleft.isSelected());
                Log.d("THMIC64", "shift button clicked, selected: " + keyshiftleft.isSelected());
                handleShiftKeys(keyshiftleft.isSelected(), ShiftKey.LEFT);
            }
        });
        keyz.setOnClickListener(view -> {
            String key = "Z";
            sendKey(context, map.get(key));
        });
        keyx.setOnClickListener(view -> {
            String key = "X";
            sendKey(context, map.get(key));
        });
        keyc.setOnClickListener(view -> {
            String key = "C";
            sendKey(context, map.get(key));
        });
        keyv.setOnClickListener(view -> {
            String key = "V";
            sendKey(context, map.get(key));
        });
        keyb.setOnClickListener(view -> {
            String key = "B";
            sendKey(context, map.get(key));
        });
        keyn.setOnClickListener(view -> {
            String key = "N";
            sendKey(context, map.get(key));
        });
        keym.setOnClickListener(view -> {
            String key = "M";
            sendKey(context, map.get(key));
        });
        keycomma.setOnClickListener(view -> {
            String key = ",";
            sendKey(context, map.get(key));
        });
        keyperiod.setOnClickListener(view -> {
            String key = ".";
            sendKey(context, map.get(key));
        });
        keyslash.setOnClickListener(view -> {
            String key = "/";
            sendKey(context, map.get(key));
        });
        keyshiftright.setOnClickListener(view -> {
            if (settings.isSendRawKeyCodes()) {
                String key = "shiftright";
                sendKey(context, map.get(key));
            } else {
                if (keyshiftlock.isSelected()) {
                    return;
                }
                keyshiftright.setSelected(!keyshiftright.isSelected());
                Log.d("THMIC64", "shift button clicked, selected: " + keyshiftright.isSelected());
                handleShiftKeys(keyshiftright.isSelected(), ShiftKey.RIGHT);
            }
        });
        keycrsrdown.setOnClickListener(view -> {
            String key = "crsrdown";
            sendKey(context, map.get(key));
        });
        keycrsrup.setOnClickListener(view -> {
            String key = "crsrup";
            sendKey(context, map.get(key));
        });
        keycrsrright.setOnClickListener(view -> {
            String key = "crsrright";
            sendKey(context, map.get(key));
        });
        keycrsrleft.setOnClickListener(view -> {
            String key = "crsrleft";
            sendKey(context, map.get(key));
        });
        keyspace.setOnClickListener(view -> {
            String key = "space";
            sendKey(context, map.get(key));
        });
    }
}
