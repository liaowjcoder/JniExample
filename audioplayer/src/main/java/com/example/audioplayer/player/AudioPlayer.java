package com.example.audioplayer.player;

import android.text.TextUtils;
import android.util.Log;

/**
 * Created by zeal on 2018/11/23.
 */

public class AudioPlayer {
    static {
        System.loadLibrary("audio-lib");
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
    private OnPlayLoadListener mOnPlayLoadListener = null;
    private OnPlayTimeInfoListener mOnPlayTimeInfoListener = null;
    private OnPlayResumeAndPauseListener mOnPlayResumeAndPauseListener = null;
    private OnPlayErrorListener mOnPlayErrorListener = null;
    private OnPlayCompleteListener mOnPlayCompleteListener = null;
    private String source = null;
    private boolean playNext = false;

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
        //准备阶段--回调加载中
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
        _start();
    }

    public void pause() {
        _pause();
        if (mOnPlayResumeAndPauseListener != null) {
            mOnPlayResumeAndPauseListener.onPause();
        }
    }

    public void onResume() {
        _resume();

        if (mOnPlayResumeAndPauseListener != null) {
            mOnPlayResumeAndPauseListener.onResume();
        }
    }

    public void stop() {
        new Thread() {
            @Override
            public void run() {
                super.run();
                _stop();
            }
        }.start();
    }

    public void seek(int seconds) {
        _seek(seconds);
    }


    public void playNext(String nextUrl) {
        this.source = nextUrl;//更新当前的 url
        playNext = true;//标记要播放下一首歌
        stop();
    }


    public void setOnPlayPreparedListener(OnPlayPreparedListener onPlayPreparedListener) {
        this.mOnPlayPreparedListener = onPlayPreparedListener;
    }


    public void setOnPlayLoadListener(OnPlayLoadListener onPlayLoadListener) {
        this.mOnPlayLoadListener = onPlayLoadListener;
    }

    public void setOnPlayTimeInfoListener(OnPlayTimeInfoListener onPlayTimeInfoListener) {
        this.mOnPlayTimeInfoListener = onPlayTimeInfoListener;
    }


    public void setOnPlayResumeAndPauseListener(OnPlayResumeAndPauseListener onPlayResumeAndPauseListener) {
        this.mOnPlayResumeAndPauseListener = onPlayResumeAndPauseListener;
    }


    public void setOnPlayErrorListener(OnPlayErrorListener onPlayErrorListener) {
        this.mOnPlayErrorListener = onPlayErrorListener;
    }

    public void setOnPlayCompleteListener(OnPlayCompleteListener onPlayCompleteListener) {
        this.mOnPlayCompleteListener = onPlayCompleteListener;
    }


    public void onCallPrepared() {
        //c++层回调java层，表示 ffmpeg已经准备完毕，可以是播放了
        if (mOnPlayPreparedListener != null) {
            mOnPlayPreparedListener.onPrepared();
        }
    }

    public void onCallOnLoad(boolean onLoad) {
        if (mOnPlayLoadListener != null) {
            mOnPlayLoadListener.onLoad(onLoad);
        }
    }

    public void onCallTimeInfo(int currentTime, int duaration) {

        TimeInfo timeInfo = new TimeInfo(currentTime, duaration);

        if (mOnPlayTimeInfoListener != null) {
            mOnPlayTimeInfoListener.onPlayTimeInfo(timeInfo);
        }
    }

    public void onCallError(int errCode, String errMsg) {

        stop();

        if (mOnPlayErrorListener != null) {
            mOnPlayErrorListener.onError(errCode, errMsg);
        }
    }

    public void onCallComplete() {

        stop();

        if (mOnPlayCompleteListener != null) {
            mOnPlayCompleteListener.onComplete();
        }
    }

    public void onCallPlayNext() {
        if (playNext) {
            playNext = false;
            prepare();
        }
    }

    /**
     * 准备
     *
     * @param source
     */
    private native void _prepare(String source);

    private native void _start();

    private native void _pause();

    private native void _resume();

    private native void _stop();

    private native void _seek(int seconds);


}

