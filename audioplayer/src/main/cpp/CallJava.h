//
// Created by 廖伟健 on 2018/11/23.
//

#ifndef JNIEXAMPLE_CALLJAVA_H
#define JNIEXAMPLE_CALLJAVA_H

#define MAIN_THREAD 0
#define CHILD_THREAD 1

#include "jni.h"
#include "AndroidLog.h"
#include "string.h"

class CallJava {
public:
    JNIEnv *jniEnv;
    JavaVM *javaVM;
    jobject jobj;
    jmethodID m_prepare_id;
    jmethodID m_call_time_info;
    jmethodID m_call_onload;
    jmethodID m_call_error;

public:
    CallJava(JNIEnv *jniEnv, JavaVM *javaVM, jobject *jobj);


    ~CallJava();

    void onCallPrepared(int type);

    void onCallTimeInfo(int type, int currentTime, int duration);

    void onCallOnLoad(int type, jboolean isLoad);

    void onCallError(int type,int errCode,char* errMsg);

};


#endif //JNIEXAMPLE_CALLJAVA_H
