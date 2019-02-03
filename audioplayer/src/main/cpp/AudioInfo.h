//
// Created by 廖伟健 on 2018/11/25.
//

#ifndef JNIEXAMPLE_AUDIOINFO_H
#define JNIEXAMPLE_AUDIOINFO_H

extern "C" {
#include "libavcodec/avcodec.h"
#include <libswresample/swresample.h>
//opensles
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};

#include "PacketQueue.h"
#include "PlayStatus.h"
#include "CallJava.h"

class AudioInfo {
public:
    //流ID值
    int streamIndex = -1;
    //解码器上下文
    AVCodecContext *avCodecContext = NULL;
    //解码器相关参数信息
    AVCodecParameters *avCodecParameters = NULL;

    //存放解码获取的AvPacket
    PacketQueue *packetQueue = NULL;

    //播放状态
    PlayStatus *playStatus = NULL;
    //播放线程
    pthread_t pthreadPlay;
    //当前解码的 AvPacket
    AVPacket *avPacket = NULL;
    //当前解码出来的AvFrame
    AVFrame *avFrame = NULL;
    //重采样的缓冲区
    u_int8_t *resampleBuffer = NULL;
    //当前重采样的大小
    int dataSize = 0;
    //返回值
    int ret = -1;

    //opensles
    SLEngineItf engineItf;
    SLObjectItf engineObj;
    SLObjectItf outputMixObject = NULL;
    SLObjectItf playerObj = NULL;
    SLPlayItf playItf = NULL;
    SLAndroidSimpleBufferQueueItf bufferQueueItf = NULL;

    CallJava *callJava = NULL;


    int sampleRate;
    //歌曲时间相关
    int duration = 0;
    AVRational time_base;
    double clock = 0;
    double current_time = 0;//当前时间
    double last_time = 0;//上一次的时间

public:
    AudioInfo(CallJava *callJava, int sampleRate, PlayStatus *playStatus);

    ~AudioInfo();

    void play();

    SLEngineItf createEnginIntf();

    int resample();

    void initOpenSLES();

    void pause();

    void resume();

    int getCurrentSampleRateForOpensles(int sample_rate);

    void stop();

    void release();
};


#endif //JNIEXAMPLE_AUDIOINFO_H
