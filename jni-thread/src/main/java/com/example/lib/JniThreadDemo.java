package com.example.lib;

/**
 * Created by zeal on 2018/11/8.
 */

public class JniDemo {

    static {
        System.loadLibrary("native-lib");

    }


    public native void createThread();
}
