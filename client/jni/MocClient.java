package com.hunantv.mongoq;

import android.content.res.AssetManager;
/**
 * Created with IntelliJ IDEA.
 * User: bigclean
 * Date: 2013/11/11
 * Time: 17:56
 */
public class MocClient {
    /**
     * @todo add 'registCallback' interface for client.
     */
    public native boolean init(AssetManager assetManager, String file);

    public native int trigger(String module, String key, short cmd, short flags);

    public native void setParam(String module, String key, String val);

    public native void destroy();
}
