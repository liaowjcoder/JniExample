package com.example.lib;

import android.util.Log;

/**
 * Created by zeal on 2018/11/8.
 */

public class JniThreadDemo {

    private static final String TAG = JniThreadDemo.class.getSimpleName();

    static {
        System.loadLibrary("native-thread-lib");
    }

    public native void createThread();

    public native void callJavaMethodOnCPPMainThread();

    public native void callJavaMethodOnCppChildThread();

    public void onSuccess(String msg) {
        Log.i(TAG, "JNI 回调 Java层 onSuccess 方法:" + msg);
    }

    public Animal search(int age,String name){return null;}

    public boolean compare(Animal animal1,Animal animal2){return false;}

    public boolean compare(int arr[],Animal[] animals){return false;}
}
