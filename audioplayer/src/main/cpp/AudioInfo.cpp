//
// Created by liaoweijian on 2018/11/25.
//

#include "AudioInfo.h"

AudioInfo::AudioInfo(PlayStatus *playStatus) {
    this->packetQueue = new PacketQueue(playStatus);
}

AudioInfo::~AudioInfo() {

}
