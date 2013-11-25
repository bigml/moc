package com.hunantv.tazai;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.content.res.AssetManager;
import android.util.Log;
import android.widget.TextView;

import org.clearsilver.HDF;

public class BangActivity extends Activity {
    static AssetManager assetManager;

    private TextView infoView;
    private Handler  handler;
    private HDF      dataNode;
    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        infoView     = (TextView) findViewById(R.id.info_view);
        handler      = new Handler();
        assetManager = getAssets();

        init(assetManager, null);
        registCallback("bang");
        // login to moc server
        setParam("bang", "userid", "tazai");
        trigger("bang", null, (short) 1001, (short) 0);
        // send battle request to server
        trigger("bang", null, (short) 1011, (short) 0);
    }

    public void loginCallback(String dataStr) {
        dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", dataNode.dump());

        handler.post(loginCallbackThread);
    }

    Runnable loginCallbackThread = new Runnable() {
        @Override
        public void run() {
            infoView.setText(dataNode.dump());
        }
    };

    public void quitCallback(String dataStr) {
        dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", dataNode.getValue("userid", null) + " quits.");

        handler.post(quitCallbackThread);
    }

    Runnable quitCallbackThread = new Runnable() {
        @Override
        public void run() {
            infoView.setText(dataNode.getValue("userid", null) + " quits.");
        }
    };

    public void battleInviteCallback(String dataStr) {
        dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", "receive battle invite from table " + dataNode.getIntValue("tableid", 0));

        handler.post(battleInviteCallbackThread);
    }

    Runnable battleInviteCallbackThread = new Runnable() {
        @Override
        public void run() {
            infoView.setText("receive battle invite from table " + dataNode.getIntValue("tableid", 0));

            // it always accept battle request in demo
            trigger("bang", null, (short) 1012, (short) 0);
        }
    };

    public void battleBeginCallback(String dataStr) {
        dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", "receive begin battle info from server.");
        Log.i("moc", dataNode.dump());

        handler.post(battleBeginCallbackThread);
    }

    Runnable battleBeginCallbackThread = new Runnable() {
        @Override
        public void run() {
            infoView.setText(dataNode.dump());
        }
    };

    public void joinCallback(String dataStr) {
        dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", dataNode.getValue("userid", null) + ": " + dataNode.getValue("direction", null));

        handler.post(joinCallbackThread);
    }

    Runnable joinCallbackThread = new Runnable() {
        @Override
        public void run() {
            infoView.setText(dataNode.getValue("userid", null) + ": " + dataNode.getValue("direction", null));
        }
    };

    public void turnCallback(String dataStr) {
        dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", dataNode.getValue("userid", null) + ": " + dataNode.getValue("redirection", null));

        handler.post(turnCallbackThread);
    }

    Runnable turnCallbackThread = new Runnable() {
        @Override
        public void run() {
            infoView.setText(dataNode.getValue("userid", null) + ": " + dataNode.getValue("redirection", null));
        }
    };

    public native boolean init(AssetManager assetManager, String file);

    public native void registCallback(String module);

    public native int trigger(String module, String key, short cmd, short flags);

    public native void setParam(String module, String key, String val);

    public native void destroy();

    // Load native moc client library
    static {
        System.loadLibrary("moc");
    }
}
