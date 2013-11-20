package com.hunantv.tazai;

import android.content.res.AssetManager;
import android.util.Log;

import org.clearsilver.HDF;

/**
 * Encapsulated class of moc client.
 * Date: 2013/11/11
 * Time: 17:56
 */
public class MocClient {
    public native boolean init(AssetManager assetManager, String file);

    public native void registCallback();

    public native int trigger(String module, String key, short cmd, short flags);

    public native void setParam(String module, String key, String val);

    public native void destroy();

    public void loginCallback(HDF dataNode) {
        Log.i("moc", dataNode.dump());
    }
}