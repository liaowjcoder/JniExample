//
// Created by 廖伟健 on 2018/11/23.
//

#include "CallJava.h"

CallJava::CallJava(JNIEnv *jniEnv, JavaVM *javaVM, jobject *jobj) {
    this->jniEnv = jniEnv;
    this->javaVM = javaVM;
    this->jobj = *jobj;
    this->jobj = jniEnv->NewGlobalRef(this->jobj);

    jclass jclz = jniEnv->GetObjectClass(this->jobj);
    m_prepare_id = jniEnv->GetMethodID(jclz, "onCallPrepared", "()V");

    m_call_time_info = jniEnv->GetMethodID(jclz, "onCallTimeInfo", "(II)V");
    m_call_onload = jniEnv->GetMethodID(jclz, "onCallOnLoad", "(Z)V");
}

void CallJava::onCallPrepared(int type) {
    switch (type) {
        case MAIN_THREAD://在主线程调用
            jniEnv->CallVoidMethod(jobj, m_prepare_id);
            break;
        case CHILD_THREAD://在子线程调用
            JNIEnv *childJniEnv = NULL;

            if (javaVM->AttachCurrentThread(&childJniEnv, 0) != JNI_OK) {
                LOGE("attach thread error...");
                return;
            }

            childJniEnv->CallVoidMethod(jobj, m_prepare_id);

            javaVM->DetachCurrentThread();

            break;
    }
}

void CallJava::onCallTimeInfo(int type, int currentTime, int duration) {
    switch (type) {
        case MAIN_THREAD:
            jniEnv->CallVoidMethod(jobj, m_call_time_info, currentTime, duration);
            break;
        case CHILD_THREAD:

            JNIEnv *childJniEnv = NULL;
            if (javaVM->AttachCurrentThread(&childJniEnv, 0) != JNI_OK) {
                LOGE("attach thread error...")
                return;
            }

            childJniEnv->CallVoidMethod(jobj, m_call_time_info, currentTime, duration);

            javaVM->DetachCurrentThread();
            break;
    }
}

void CallJava::onCallOnLoad(int type, jboolean isLoad) {


    switch (type) {
        case MAIN_THREAD:
            jniEnv->CallVoidMethod(jobj, m_call_onload, isLoad);
            break;
        case CHILD_THREAD:
            JNIEnv *childJniEnv = NULL;

            if (javaVM->AttachCurrentThread(&childJniEnv, 0) != JNI_OK) {
                LOGE("attach thread error...")
                return;
            }

            childJniEnv->CallVoidMethod(jobj, m_call_onload, isLoad);

            javaVM->DetachCurrentThread();
            break;
    }

}

CallJava::~CallJava() {

}
