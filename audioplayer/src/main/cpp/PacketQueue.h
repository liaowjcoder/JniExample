//
// Created by liaoweijian on 2018/11/28.
//

#ifndef JNIEXAMPLE_PACKETQUEUE_H
#define JNIEXAMPLE_PACKETQUEUE_H

#include "pthread.h"
#include "queue"
#include "AndroidLog.h"
#include "PlayStatus.h"

extern "C" {
#include "libavcodec/avcodec.h"
};

class PacketQueue {
public:

    std::queue<AVPacket *> queueAvpacket;
    pthread_cond_t condAvPacket;
    pthread_mutex_t mutexAvPacket;
    PlayStatus *playStatus = NULL;
public :
    PacketQueue(PlayStatus *playStatus);

    ~PacketQueue();

    int putAvPacket(AVPacket *avPacket);

    int getAvPacket(AVPacket *avPacket);

    int getAvPacketQueueSize();
};


#endif //JNIEXAMPLE_PACKETQUEUE_H
