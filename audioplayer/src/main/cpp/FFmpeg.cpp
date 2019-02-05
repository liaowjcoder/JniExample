//
// Created by 廖伟健 on 2018/11/23.
//



#include "FFmpeg.h"




FFmpeg::FFmpeg(PlayStatus *playStatus, CallJava *callJava, const char *url) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    this->url = url;
    exit = false;
    pthread_mutex_init(&mutex_init, NULL);
    pthread_mutex_init(&mutext_seek, NULL);
}

FFmpeg::~FFmpeg() {
    pthread_mutex_destroy(&mutex_init);
    pthread_mutex_destroy(&mutext_seek);
}

void *callbackDecode(void *data) {
    FFmpeg *ffmpeg = (FFmpeg *) data;

    ffmpeg->decodeFFmpegThread();

    pthread_exit(&ffmpeg->decodeThread);
}

void FFmpeg::prepare() {
    pthread_create(&decodeThread, NULL, callbackDecode, this);
}

//会频繁调用，在这里你可以做一个退出操作。
int interrupt_callback(void *ctx) {
    FFmpeg *fFmpeg = static_cast<FFmpeg *>(ctx);
//    LOGD("interrupt_callback")
    if (fFmpeg->playStatus->isExit) {
        return 1;
    }
    return 0;
}


/**
 * 在子线程中调用
 */
void FFmpeg::decodeFFmpegThread() {
    pthread_mutex_lock(&mutex_init);
    //注册
    av_register_all();
    avformat_network_init();

    //打开文件或网络流
    avFormatContext = avformat_alloc_context();
    //设置中断

    avFormatContext->interrupt_callback.callback = interrupt_callback;
    avFormatContext->interrupt_callback.
            opaque = this;

    if (int ret = avformat_open_input(&avFormatContext, url, NULL, NULL) != 0) {
        LOGE("avformat_open_input failed...%s %s", av_err2str(ret), url);
        exit = true;
        callJava->onCallError(CHILD_THREAD, 1000, "avformat_open_input failed");
        pthread_mutex_unlock(&mutex_init);
        return;
    }


    //获取流信息
    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        LOGE("avformat_find_stream_info failed...");
        exit = true;
        callJava->onCallError(CHILD_THREAD, 1001, "avformat_find_stream_info failed...");
        pthread_mutex_unlock(&mutex_init);
        return;
    }


    //获取音频流
    for (int i = 0; i < avFormatContext->nb_streams; i++) {
        //找到对应的音频流信息
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audioInfo == NULL) {
                //创建 AudioInfo 保存音频相关信息
                audioInfo = new AudioInfo(callJava,
                                          avFormatContext->streams[i]->codecpar->sample_rate,
                                          playStatus);
                audioInfo->streamIndex = i;
                audioInfo->avCodecParameters = avFormatContext->streams[i]->codecpar;
                //时间基
                audioInfo->time_base = avFormatContext->streams[i]->time_base;
                //总时长
                audioInfo->duration = avFormatContext->duration / AV_TIME_BASE;
                break;
            }
        }
    }

    //根据 AVCodecID 获取解码器
    const AVCodec *avCodec = avcodec_find_decoder(audioInfo->avCodecParameters->codec_id);
    if (!avCodec) {
        LOGE("avcodec_find_decoder failed...");
        callJava->onCallError(CHILD_THREAD, 1002, "avcodec_find_decoder failed...");
        exit = true;
        pthread_mutex_unlock(&mutex_init);
        return;
    }
    //利用解码器创建解码器上下文
    audioInfo->avCodecContext = avcodec_alloc_context3(avCodec);

    if (!audioInfo->avCodecContext) {
        LOGE("avcodec_alloc_context3 failed...");
        callJava->onCallError(CHILD_THREAD, 1003, "avcodec_alloc_context3 failed...");
        exit = true;
        pthread_mutex_unlock(&mutex_init);
        return;
    }
    if (avcodec_parameters_to_context(audioInfo->avCodecContext, audioInfo->avCodecParameters) <
        0) {
        LOGE("avcodec_parameters_to_context failed...");
        exit = true;
        callJava->onCallError(CHILD_THREAD, 1004, "avcodec_parameters_to_context failed...");
        pthread_mutex_unlock(&mutex_init);
        return;
    }
    //打开解码器
    if (avcodec_open2(audioInfo->avCodecContext, avCodec, 0) != 0) {
        LOGE("avcodec_open2 failed...");
        exit = true;
        callJava->onCallError(CHILD_THREAD, 1005, "avcodec_open2 failed...");
        pthread_mutex_unlock(&mutex_init);
        return;
    }

    //准备完毕，可以告诉上层，音频准备完毕，可以开始播放了。
    //可能这时其他线程出发了 release 方法，导致 playStatus->isExit = true
    if (callJava != NULL && !playStatus->isExit) {
        callJava->onCallPrepared(CHILD_THREAD);
    } else {
        exit = true;
    }

    pthread_mutex_unlock(&mutex_init);
}

void FFmpeg::start() {
    if (audioInfo == NULL) {
        LOGE("start failed audio info is null.")
        callJava->onCallError(CHILD_THREAD, 1006, "start failed audio info is null");
        return;
    }

    //播放音频线程
    //开始重采样音频数据
    //一开始getAvPacket没有数据时线程会等待。
    audioInfo->play();

    int count = 0;
    //解码
    while (playStatus != NULL && !playStatus->isExit) {

        if (playStatus->seek) {
            continue;
        }


        //解封装完毕，将数据放入到队里，如果此时 seek就会把队列情况。这时走 else 逻辑，就会导致 playstatus->exit = true; 直接退出。
        //保证队列的数据不大于40个。减少内存的压力。
        if (audioInfo->packetQueue->getAvPacketQueueSize() > 40) {
            continue;
        }


        AVPacket *avPacket = av_packet_alloc();




        //确保操作 avFormatContext 只有一个线程，避免 seek 操作。
        pthread_mutex_lock(&mutext_seek);
        int ret = av_read_frame(avFormatContext, avPacket);
        pthread_mutex_unlock(&mutext_seek);

        if (ret == 0) {

            if (avPacket->stream_index == audioInfo->streamIndex) {
                count++;
//                LOGD("当前解码第%d帧", count);
//                av_packet_free(&avPacket);
//                av_free(avPacket);
//                avPacket = NULL;
                audioInfo->packetQueue->putAvPacket(avPacket);
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
            }
        } else {
            LOGD("解封装完成，总共解码%d帧", count);
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            while (playStatus != NULL && !playStatus->isExit) {
                if (audioInfo->packetQueue->getAvPacketQueueSize() > 0) {
                    //解封装完毕，队列还有数据，还需要等待队列数据播放完毕
                    continue;
                } else {
                    //真正的结束了
                    playStatus->isExit = true;
                }
            }
            break;
        }
    }

    //代码走到这里表示 playStatus->isExit = true 要退出了。
    if (callJava != NULL) {
        LOGD("onCallComplete");
        callJava->onCallComplete(CHILD_THREAD);
    }

    //解封装完毕
    exit = true;
//    LOGD("解封装完毕:%d", count);


    //测试代码
//    while (audioInfo->packetQueue->getAvPacketQueueSize() > 0) {
//        AVPacket *avPacket = av_packet_alloc();
//
//        audioInfo->packetQueue->getAvPacket(avPacket);
//
//        av_packet_free(&avPacket);
//        av_free(avPacket);
//        avPacket = NULL;
//    }
//    LOGD("ookokookokokoko")
}

void FFmpeg::pause() {
    if (audioInfo != NULL) {
        audioInfo->pause();
    }
}

void FFmpeg::resume() {
    if (audioInfo != NULL) {
        audioInfo->resume();
    }
}

void FFmpeg::release() {

    LOGD("开始释放 ffmpeg")




    playStatus->isExit = true;

    int sleepCount = 0;
    pthread_mutex_lock(&mutex_init);
    while (!exit) {

        if (sleepCount > 1000) {
            exit = true;
        }

        LOGE("wait exit %d", sleepCount)

        sleepCount++;
        av_usleep(1000 * 10);//睡眠10ms
    }
    pthread_mutex_unlock(&mutex_init);
    LOGD("开始释放 audioInfo")
    //释放 AudioInfo
    if (audioInfo != NULL) {

        audioInfo->release();
        delete (audioInfo);
        audioInfo = NULL;
    }

    LOGD("开始释放 avFormatContext")
    if (avFormatContext != NULL) {
        avformat_close_input(&avFormatContext);
        avformat_free_context(avFormatContext);
        avFormatContext = NULL;
    }
    LOGD("开始释放 playStatus")
    //只需要将当前的指针置 NULL 即可，不能 delete 因为其他的类会是用到这个对象
    if (playStatus != NULL) {
        playStatus = NULL;
    }
    LOGD("开始释放 callJava")
    if (callJava != NULL) {
        callJava = NULL;
    }


}

void FFmpeg::seek(int seconds) {
    if (seconds <= 0) {
        return;
    }
    LOGD("seek1")
    if (audioInfo != NULL) {
        if (audioInfo->duration > 0 && seconds <= audioInfo->duration) {
            LOGD("seek2")
            playStatus->seek = true;
            audioInfo->packetQueue->clearAvPacket();
            audioInfo->clock = 0;
            audioInfo->last_time = 0;

            //这里在 seek 的时候，其他线程可能正在操作 read_av_frame()，因此avFormatContext操作需要加锁。
            pthread_mutex_lock(&mutext_seek);
            LOGD("seek3")
            avformat_seek_file(avFormatContext, -1, INT64_MIN, seconds * AV_TIME_BASE, INT64_MAX,
                               0);
            LOGD("seek4")
            pthread_mutex_unlock(&mutext_seek);

            playStatus->seek = false;

        }

    }
}

