package com.hunantv.tazai;

import android.app.Activity;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;

import org.clearsilver.HDF;

public class BangActivity extends Activity {
    static AssetManager assetManager;
    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        assetManager = getAssets();

        MocClient.init(assetManager, null);
        MocClient.registCallback("bang");
        // login to moc server, then send battle request once.
        MocClient.setParam("bang", "userid", "tazai");
        MocClient.trigger("bang", null, (short) 1001, (short) 0);
        MocClient.trigger("bang", null, (short) 1011, (short) 0);
        //MocClient.destroy();
    }

    public static void loginCallback(String dataStr) {
        HDF dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", dataNode.dump());
    }

    public static void quitCallback(String dataStr) {
        HDF dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", dataNode.getValue("userid", null) + " quits.");
    }

    public static void battleInviteCallback(String dataStr) {
        HDF dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", "receive battle invite from table " + dataNode.getIntValue("tableid", 0));
        // it always accept battle request in demo
        MocClient.trigger("bang", null, (short) 1012, (short) 0);
    }

    public static void battleBeginCallback(String dataStr) {
        HDF dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", "receive begin battle info from server.");
        Log.i("moc", dataNode.dump());
    }

    public static void joinCallback(String dataStr) {
        HDF dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", dataNode.getValue("userid", null) + ": " + dataNode.getValue("direction", null));
    }

    public static void turnCallback(String dataStr) {
        HDF dataNode = new HDF();
        dataNode.readString(dataStr);
        Log.i("moc", dataNode.getValue("userid", null) + ": " + dataNode.getValue("redirection", null));
    }

    // Load native moc client library
    static {
        System.loadLibrary("moc");
    }
}
