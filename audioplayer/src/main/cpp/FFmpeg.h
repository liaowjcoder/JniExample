//
// Created by 廖伟健 on 2018/11/23.
//

#ifndef JNIEXAMPLE_FFMPEG_H
#define JNIEXAMPLE_FFMPEG_H

#include <pthread.h>

#include "CallJava.h"
#include "AndroidLog.h"
#include "AudioInfo.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

#include "PlayStatus.h"

class FFmpeg {

public:

    CallJava *callJava = NULL;
    pthread_t decodeThread;
    const char *url = NULL;
    AVFormatContext *avFormatContext = NULL;
    AudioInfo *audioInfo = NULL;
    PlayStatus *playStatus = NULL;


    //是否退出
    bool exit;


    pthread_mutex_t mutex_init;
    pthread_mutex_t mutext_seek;

public:
    FFmpeg(PlayStatus *playStatus, CallJava *callJava, const char *url);

    ~FFmpeg();

    void prepare();

    void decodeFFmpegThread();

    void start();

    void pause();

    void resume();

    void release();

    void seek(int seconds);

    void setVolume(int percent);
};


#endif //JNIEXAMPLE_FFMPEG_H
