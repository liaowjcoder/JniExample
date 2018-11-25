//
// Created by 廖伟健 on 2018/11/23.
//


#include "FFmpeg.h"

FFmpeg::FFmpeg(CallJava *callJava, const char *url) {
    this->callJava = callJava;
    this->url = url;
}

void *callbackDecode(void *data) {
    FFmpeg *ffmpeg = (FFmpeg *) data;

    ffmpeg->decodeFFmpegThread();

    pthread_exit(&ffmpeg->decodeThread);
}

void FFmpeg::prepare() {
    pthread_create(&decodeThread, NULL, callbackDecode, this);
}

/**
 * 在子线程中调用
 */
void FFmpeg::decodeFFmpegThread() {

    //注册
    av_register_all();
    avformat_network_init();

    //打开文件或网络流
    avFormatContext = avformat_alloc_context();

    if (avformat_open_input(&avFormatContext, url, NULL, NULL) != 0) {
        LOGE("avformat_open_input failed...");
        return;
    }


    //获取流信息
    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        LOGE("avformat_find_stream_info failed...");
        return;
    }


    //获取音频流
    for (int i = 0; i < avFormatContext->nb_streams; i++) {
        //找到对应的音频流信息
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audioInfo == NULL) {
                //创建 AudioInfo 保存音频相关信息
                audioInfo = new AudioInfo();
                audioInfo->streamIndex = i;
                audioInfo->avCodecParameters = avFormatContext->streams[i]->codecpar;
                break;
            }
        }
    }

    //根据 AVCodecID 获取解码器
    const AVCodec *avCodec = avcodec_find_decoder(audioInfo->avCodecParameters->codec_id);
    if (!avCodec) {
        LOGE("avcodec_find_decoder failed...");
        return;
    }
    //利用解码器创建解码器上下文
    audioInfo->avCodecContext = avcodec_alloc_context3(avCodec);

    if (!audioInfo->avCodecContext) {
        LOGE("avcodec_alloc_context3 failed...");
        return;
    }
    if (avcodec_parameters_to_context(audioInfo->avCodecContext, audioInfo->avCodecParameters) <
        0) {
        LOGE("avcodec_parameters_to_context failed...");
        return;
    }
    //打开解码器
    if (avcodec_open2(audioInfo->avCodecContext, avCodec, 0) != 0) {
        LOGE("avcodec_open2 failed...");
        return;
    }

    //准备完毕，可以告诉上层，音频准备完毕，可以开始播放了。
    callJava->onCallPrepared(CHILD_THREAD);
}

void FFmpeg::start() {
    if (audioInfo == NULL) {
        LOGE("start failed audio info is null.")
        return;
    }

    int count = 0;

    while (1) {
        AVPacket *avPacket = av_packet_alloc();

        if (av_read_frame(avFormatContext, avPacket) == 0) {

            if (avPacket->stream_index == audioInfo->streamIndex) {
                count++;
                LOGD("当前解码第%d帧", count);
                av_packet_free(&avPacket);
                av_free(avPacket);
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else {
            LOGD("解码完成，总共解码%d帧", count);
            av_packet_free(&avPacket);
            av_free(avPacket);
            break;
        }
    }
}
