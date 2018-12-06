#include <jni.h>
#include <string>

#include "AndroidLog.h"

extern "C" {
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};

SLEngineItf engineItf;

SLEngineItf createEnginIntf() {

    SLObjectItf engineObj;
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

void callback(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
    LOGD("callback...");

    static FILE *fp;
    static char *buffer;


    if (!buffer) {
        buffer = new char[1024 * 1024];
    }

    if (!fp) {
        fp = fopen("/mnt/sdcard/loveshow_pcm.pcm", "rb");
//        fp = fopen("/mnt/sdcard/mydream.pcm", "rb");
    }

    if (!fp) {
        LOGD("fopen failed");
        return;
    }

    if ((feof(fp)) == 0) {
        int len = fread(buffer, 1, 1024, fp);
        LOGD("READ LEN = %d", len)
        if (len > 0) {
            (*caller)->Enqueue(caller, buffer, len);
        } else {
            fclose(fp);
            fp = NULL;
            buffer = NULL;
        }
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_example_com_play_PCMPlayer__1play(JNIEnv *env, jobject instance) {
    //1.创建引擎接口
    engineItf = createEnginIntf();

    if (!engineItf) {
        LOGD("engine create failed...");
    }

    //2.创建混音器
    SLObjectItf outputMixObject = NULL;
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
            SL_SAMPLINGRATE_44_1,//SLuint32 		samplesPerSec;
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
    SLObjectItf playerObj = NULL;
    SLPlayItf playItf = NULL;
    SLAndroidSimpleBufferQueueItf bufferQueueItf = NULL;
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
    (*bufferQueueItf)->RegisterCallback(bufferQueueItf, callback, 0);
    //设置为播放状态
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    //启动队列
    (*bufferQueueItf)->Enqueue(bufferQueueItf, "", 1);
}
