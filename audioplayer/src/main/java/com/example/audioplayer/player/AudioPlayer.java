package com.example.audioplayer.player;

import android.text.TextUtils;
import android.util.Log;

/**
 * Created by zeal on 2018/11/23.
 */

public class AudioPlayer {
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

    private static final String TAG = AudioPlayer.class.getSimpleName();
    private OnPlayPreparedListener mOnPlayPreparedListener = null;
    private String source = null;

    public AudioPlayer() {
    }

    public AudioPlayer(String source) {
        this.source = source;
    }

    public void setSource(String source) {
        this.source = source;
    }

    public void prepare() {

        if (TextUtils.isEmpty(source)) {

            Log.e(TAG, "resource can't be null");
            return;
        }

        new Thread() {
            @Override
            public void run() {
                super.run();
                _prepare(source);
            }
        }.start();

    }

    public void start() {
        if (TextUtils.isEmpty(source)) {
            Log.d(TAG, "source is empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                _start();
            }
        }).start();
    }

    public void setOnPlayPreparedListener(OnPlayPreparedListener onPlayPreparedListener) {
        this.mOnPlayPreparedListener = onPlayPreparedListener;
    }

    public void onCallPrepared() {
        //c++层回调java层，表示 ffmpeg已经准备完毕，可以是播放了
        if (mOnPlayPreparedListener != null) {
            mOnPlayPreparedListener.onPrepared();
        }
    }

    /**
     * 准备
     *
     * @param source
     */
    private native void _prepare(String source);

    private native void _start();
}

