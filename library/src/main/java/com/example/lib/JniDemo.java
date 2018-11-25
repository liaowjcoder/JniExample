package com.example.lib;

/**
 * Created by zeal on 2018/11/8.
 */

public class JniDemo {

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("swresample-2");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("swscale-4");
        System.loadLibrary("postproc-54");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avdevice-57");
    }


    public native void test();
}
