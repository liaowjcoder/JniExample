#include <jni.h>
#include <string>
//
//extern "C" {
//#include <libavformat/avformat.h>
//}

#include "FFmpeg.h"

JavaVM *jvm = NULL;
FFmpeg *ffmpeg = NULL;
CallJava *callJava = NULL;
PlayStatus *playStatus = NULL;
//native 退出
bool nexit = true;
pthread_t pthread_start;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_audioplayer_player_AudioPlayer__1prepare(JNIEnv *env, jobject instance,
                                                          jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);
    LOGD("source:%s", source)
    if (ffmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new CallJava(env, jvm, &instance);
        }
        playStatus = new PlayStatus();
        callJava->onCallOnLoad(MAIN_THREAD, true);
        ffmpeg = new FFmpeg(playStatus, callJava, source);
        ffmpeg->prepare();
    }
}

void *callback_start(void *data) {
    FFmpeg *ffmpeg = (FFmpeg *) data;
    if (ffmpeg != NULL) {
        ffmpeg->start();
    }
    pthread_exit(&pthread_start);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_audioplayer_player_AudioPlayer__1start(JNIEnv *env, jobject instance) {
    pthread_create(&pthread_start, NULL, callback_start, ffmpeg);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_audioplayer_player_AudioPlayer__1pause(JNIEnv *env, jobject instance) {

    if (ffmpeg != NULL) {
        ffmpeg->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_audioplayer_player_AudioPlayer__1resume(JNIEnv *env, jobject instance) {

    if (ffmpeg != NULL) {
        ffmpeg->resume();
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_audioplayer_player_AudioPlayer__1stop(JNIEnv *env, jobject instance) {

    if (!nexit) {
        return;
    }

    nexit = false;

    jclass jcls = env->GetObjectClass(instance);
    jmethodID onCallPlayNextMethodId = env->GetMethodID(jcls, "onCallPlayNext", "()V");


    if (ffmpeg != NULL) {
        ffmpeg->release();
        delete (ffmpeg);
        ffmpeg = NULL;

        //真正的delete
        if (callJava != NULL) {
            delete (callJava);
            callJava = NULL;
        }
        if (playStatus != NULL) {
            delete (playStatus);
            playStatus = NULL;
        }
    }
    LOGD("audio-lib 释放完毕")
    nexit = true;

    env->CallVoidMethod(instance, onCallPlayNextMethodId);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_audioplayer_player_AudioPlayer__1seek(JNIEnv *env, jobject instance,
                                                       jint seconds) {

    if (ffmpeg != NULL) {
        ffmpeg->seek(seconds);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_audioplayer_player_AudioPlayer__1setVolume(JNIEnv *env, jobject instance,
                                                            jint percent) {

    if (ffmpeg != NULL) {
        ffmpeg->setVolume(percent);
    }
}


//在加载动态库时回去调用 JNI_Onload 方法，在这里可以得到 JavaVM 实例对象
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    jvm = vm;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}


