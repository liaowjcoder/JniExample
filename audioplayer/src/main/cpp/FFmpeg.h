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
}

#include "PlayStatus.h"

class FFmpeg {

public:

    CallJava *callJava = NULL;
    pthread_t decodeThread = NULL;
    const char *url = NULL;
    AVFormatContext *avFormatContext = NULL;
    AudioInfo *audioInfo = NULL;
    PlayStatus *playStatus = NULL;

public:
    FFmpeg(PlayStatus *playStatus, CallJava *callJava, const char *url);

    ~FFmpeg();

    void prepare();

    void decodeFFmpegThread();

    void start();
};


#endif //JNIEXAMPLE_FFMPEG_H
