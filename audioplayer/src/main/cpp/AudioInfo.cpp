//
// Created by liaoweijian on 2018/11/25.
//


#include "AudioInfo.h"

AudioInfo::AudioInfo(CallJava *callJava, int sampleRate, PlayStatus *playStatus) {
    this->sampleRate = sampleRate;
    this->callJava = callJava;
    this->playStatus = playStatus;
    this->packetQueue = new PacketQueue(playStatus);
    this->resampleBuffer = (u_int8_t *) av_malloc(sampleRate * 2 * 2);
}

AudioInfo::~AudioInfo() {
}

void *playCallback(void *data) {
    AudioInfo *audioInfo = (AudioInfo *) data;

    //重采样
    //int dataSie = audioInfo->resample();

    //初始化opensles
    audioInfo->initOpenSLES();

    pthread_exit(&audioInfo->pthreadPlay);
}

//播放
void AudioInfo::play() {

    pthread_create(&pthreadPlay, NULL, playCallback, this);

}

//保存PCM数据的文件
//FILE *savePcmFile = fopen("/mnt/sdcard/savePCM.pcm", "wb");

//重采样-在子线程中执行
int AudioInfo::resample() {

    while (playStatus != NULL && !playStatus->isExit) {


        //如果队列没有数据，那么
        if (packetQueue->getAvPacketQueueSize() == 0) {

            if (!playStatus->isLoad) {
                playStatus->isLoad = true;
                //显示当前正在加载中
                callJava->onCallOnLoad(CHILD_THREAD, true);
            }
            continue;
        } else {
            if (playStatus->isLoad) {
                playStatus->isLoad = false;
                //取消显示当前正在加载中
                callJava->onCallOnLoad(CHILD_THREAD, false);
            }
        }


//        LOGD("resample")
        avPacket = av_packet_alloc();

        //从队列中获取AvPacket
        if (packetQueue->getAvPacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        //将 AvPacket 进行解码
        ret = avcodec_send_packet(avCodecContext, avPacket);

        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            LOGE("avcodec_send_packet Error");
            continue;
        }
        //得到AvFrame
        avFrame = av_frame_alloc();

        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            LOGE("avcodec_receive_frame Error");
            continue;
        }

//        LOGD("channels=%d,channel_layout = %d", avFrame->channels, avFrame->channel_layout);
        if (avFrame->channels > 0 && avFrame->channel_layout == 0) {
            avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
        } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
            avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
        }
//        LOGD("channels2=%d,channel_layout2 = %d", avFrame->channels, avFrame->channel_layout);
        SwrContext *swrContext = swr_alloc();
        swrContext = swr_alloc_set_opts(swrContext,
                                        AV_CH_LAYOUT_STEREO,//(AV_CH_FRONT_LEFT|AV_CH_FRONT_RIGHT)
//                                        av_get_default_channel_layout(2),
                                        AV_SAMPLE_FMT_S16,//enum AVSampleFormat out_sample_fmt
                                        avFrame->sample_rate,//int out_sample_rate
                                        av_get_default_channel_layout(
                                                avFrame->channel_layout),//int64_t  in_ch_layout,
//                                        (enum AVSampleFormat) avFrame-> ,//enum AVSampleFormat in_sample_fmt
                                        (AVSampleFormat) avFrame->format,
                                        avFrame->sample_rate,// int  in_sample_rate
                                        NULL, NULL
        );

        if (!swrContext) {
            LOGE("swr_alloc_set_opts failed")
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }

        if (swr_init(swrContext) != 0) {
            LOGE("swr_init failed")
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);
            swrContext = NULL;
            continue;
        }

        int nb = swr_convert(swrContext,//struct SwrContext *s,
                             &resampleBuffer,//uint8_t **out
                             avFrame->nb_samples,//输出采样个数
                             (const uint8_t **) avFrame->data,//原始数据
                             avFrame->nb_samples//输入采样个数
        );
        if (nb < 0) {
            LOGE("swr_convert ERROR")
            continue;
        }
//        LOGD("当期swr_convert的值为%d", nb);
        int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
        dataSize = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
//        LOGE("得到重采样的数据大小：%d", dataSize);

        //当前展示时间*时间基=当前的秒数
        current_time = avFrame->pts * av_q2d(time_base);
//        LOGE("current_time:%f", current_time)
        if (current_time < clock) {
            current_time = clock;
        }

        clock = current_time;

        //得到PCM数据,保存当文件中
        //fwrite(resampleBuffer, 1, dataSize, savePcmFile);
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;

        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
        if (!swrContext) {
            swr_free(&swrContext);
            swrContext = NULL;
        }
        break;
    }
    return dataSize;
}

/**
 * 播放回调
 * @param caller
 * @param pContext
 */
void callback(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
//    LOGD("callback...");

    AudioInfo *audioInfo = (AudioInfo *) pContext;

    if (audioInfo != NULL) {
        int dataSize = audioInfo->resample();

//        LOGD("callback dataSize:%d", dataSize);

        if (dataSize > 0) {
            //计算重采样 dataSize 这么多数据需要多少s，audioInfo->sampleRate*2*2表示1s的数据量
            audioInfo->clock += dataSize / ((double) audioInfo->sampleRate * 2 * 2);
//            LOGD("clock:%f,duration:%d", audioInfo->clock, audioInfo->duration);
            if (audioInfo->clock - audioInfo->last_time > 0.1) {
                //避免重复调用，只有两个当前 clock 时间与 last_time 相隔1s才需要回调给上层

                audioInfo->last_time = audioInfo->clock;

                audioInfo->callJava->onCallTimeInfo(CHILD_THREAD, audioInfo->clock,
                                                    audioInfo->duration);
            }
            (*audioInfo->bufferQueueItf)->Enqueue(audioInfo->bufferQueueItf,
                                                  (char *) audioInfo->resampleBuffer, dataSize);
        }
    } else {
        LOGD("audioInfo is null");
    }
}

SLEngineItf AudioInfo::createEnginIntf() {


    SLresult sLresult = 0;
    sLresult = slCreateEngine(&engineObj, 0, 0, 0, 0, 0);

    if (SL_RESULT_SUCCESS != sLresult) {
        LOGE("slCreateEngine failed...");
        return NULL;
    }
    sLresult = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != sLresult) {
        LOGE("engineObj Realize failed...");
        return NULL;
    }
    SLEngineItf slEngineItf = NULL;
    sLresult = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &slEngineItf);
    if (SL_RESULT_SUCCESS != sLresult) {
        LOGE("engineObj GetInterface failed...");
        return NULL;
    }
    return slEngineItf;
}

void AudioInfo::initOpenSLES() {

    LOGD("initOpenSLES begin")

    //1.创建引擎接口
    engineItf = createEnginIntf();
    if (!engineItf) {
        LOGD("engine create failed...");
        return;
    }

    //2.创建混音器
    SLresult result = (*engineItf)->CreateOutputMix(engineItf, &outputMixObject, 0, 0, 0);

    if (SL_RESULT_SUCCESS != result) {
        LOGE("CreateOutputMix failed...")
        return;
    }
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    //SLEnvironmentalReverbItf slEnvironmentalReverbItf = NULL;
    //result = (*pMix)->GetInterface(pMix, SL_IID_ENVIRONMENTALREVERB, &slEnvironmentalReverbItf);
    //if (SL_RESULT_SUCCESS != result) {
    //    LOGE("GetMixInterface failed...")
    //    return;
    //}

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSink = {&outputMix, 0};

    //设置缓冲队列
    SLDataLocator_AndroidSimpleBufferQueue androidQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                           2};

    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//SLuint32 		formatType;//播放pcm格式的数据
            2,//SLuint32 		numChannels; //2个声道（立体声）
            getCurrentSampleRateForOpensles(sampleRate),//SLuint32 		samplesPerSec;
            SL_PCMSAMPLEFORMAT_FIXED_16,//SLuint32 		bitsPerSample;
            SL_PCMSAMPLEFORMAT_FIXED_16,//SLuint32 		containerSize;
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//SLuint32 		channelMask;
            SL_BYTEORDER_LITTLEENDIAN//SLuint32		endianness;
    };

    SLDataSource ds = {
            &androidQueue,
            &pcm
    };

    //创建播放器
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineItf)->CreateAudioPlayer(engineItf, &playerObj, &ds, &audioSink, 1, ids, req);

    if (SL_RESULT_SUCCESS != result) {
        LOGE("CreateAudioPlayer failed...")
        return;
    }

    result = (*playerObj)->Realize(playerObj, SL_BOOLEAN_FALSE);

    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioPlayer Realize failed...")
        return;
    }
    result = (*playerObj)->GetInterface(playerObj, SL_IID_PLAY, &playItf);

    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioPlayer GetInterface failed...")
        return;
    }

    result = (*playerObj)->GetInterface(playerObj, SL_IID_BUFFERQUEUE, &bufferQueueItf);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SL_IID_BUFFERQUEUE GetInterface failed...")
        return;
    }
    //设置回调函数
    (*bufferQueueItf)->RegisterCallback(bufferQueueItf, callback, this);
    //设置为播放状态
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);

    //启动队列
    //(*bufferQueueItf)->Enqueue(bufferQueueItf, "", 1);
    callback(bufferQueueItf, this);

    LOGD("initOpenSLES end")
}

/**
 * 暂停播放
 */
void AudioInfo::pause() {
    if (playItf != NULL) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PAUSED);
    }
}

/**
 * 恢复播放
 */
void AudioInfo::resume() {
    if (playItf != NULL) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    }
}

int AudioInfo::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

/**
 * 停止播放
 */
void AudioInfo::stop() {
    //调用这个方法之后，OpenSL ES 的播放回调就会停止
    if (playItf != NULL) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_STOPPED);
    }
}

void AudioInfo::release() {


    stop();

    //释放队列
    if (packetQueue != NULL) {
        delete (packetQueue);
        packetQueue = NULL;
    }


    //释放 OpenES SL
    if (playerObj != NULL) {
        (*playerObj)->Destroy(playerObj);
        playerObj = NULL;
        playItf = NULL;
        bufferQueueItf = NULL;
    }

    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
    }

    if (engineItf != NULL) {
        (*engineObj)->Destroy(engineObj);
    }

    if (resampleBuffer != NULL) {
        free(resampleBuffer);
        resampleBuffer = NULL;
    }

    if(avCodecContext!=NULL){
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }

    //只需要将当前的指针置 NULL 即可，不能 delete 因为其他的类会是用到这个对象
    if(playStatus!=NULL){
        playStatus = NULL;
    }

    if(callJava!=NULL){
        callJava = NULL;
    }

}
