//
// Created by 廖伟健 on 2018/11/25.
//

#ifndef JNIEXAMPLE_AUDIOINFO_H
#define JNIEXAMPLE_AUDIOINFO_H

extern "C" {
#include "libavcodec/avcodec.h"
};

#include "PacketQueue.h"
#include "PlayStatus.h"

class AudioInfo {
public:
    //流ID值
    int streamIndex = -1;
    //解码器上下文
    AVCodecContext *avCodecContext = NULL;
    //解码器相关参数信息
    AVCodecParameters *avCodecParameters = NULL;

    PacketQueue *packetQueue = NULL;


public:
    AudioInfo(PlayStatus *playStatus);

    ~AudioInfo();
};


#endif //JNIEXAMPLE_AUDIOINFO_H
