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
    public static native boolean init(AssetManager assetManager, String file);

    public static native void registCallback(String module);

    public static native int trigger(String module, String key, short cmd, short flags);

    public static native void setParam(String module, String key, String val);

    public static native void destroy();
}