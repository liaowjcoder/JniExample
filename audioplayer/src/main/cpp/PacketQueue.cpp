//
// Created by liaoweijian on 2018/11/28.
//

#include "PacketQueue.h"

PacketQueue::PacketQueue(PlayStatus *playStatus) {
    this->playStatus = playStatus;
    pthread_mutex_init(&mutexAvPacket, 0);
    pthread_cond_init(&condAvPacket, 0);
}

PacketQueue::~PacketQueue() {
    pthread_mutex_destroy(&mutexAvPacket);
    pthread_cond_destroy(&condAvPacket);
}

int PacketQueue::putAvPacket(AVPacket *avPacket) {

    pthread_mutex_lock(&mutexAvPacket);
    queueAvpacket.push(avPacket);
    LOGD("存放一个AvPacket到队列中，当前队列还有%d个数据", queueAvpacket.size());
    //通知消费者去取数据
    pthread_cond_signal(&condAvPacket);
    pthread_mutex_unlock(&mutexAvPacket);
    return 0;
}

int PacketQueue::getAvPacket(AVPacket *avPacket) {

    pthread_mutex_lock(&mutexAvPacket);

    while (playStatus != NULL && !playStatus->isExit) {
        if (queueAvpacket.size() > 0) {
            AVPacket *packet = queueAvpacket.front();
            if (av_packet_ref(avPacket, packet) == 0) {
                //将 packet 从队列中取出来
                queueAvpacket.pop();
                LOGD("从队列中取出一个AvPacket，还剩余%d个", queueAvpacket.size());
            } else {
                pthread_cond_wait(&condAvPacket, &mutexAvPacket);
            }
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            break;
        } else {
            pthread_cond_wait(&condAvPacket, &mutexAvPacket);
        }
    }
    pthread_mutex_unlock(&mutexAvPacket);
    return 0;
}

int PacketQueue::getAvPacketQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutexAvPacket);
    size = queueAvpacket.size();
    pthread_mutex_unlock(&mutexAvPacket);
    return size;

}

