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

public:
    CallJava(JNIEnv *jniEnv, JavaVM *javaVM, jobject* jobj);


    ~CallJava();

    void onCallPrepared(int type);

};


#endif //JNIEXAMPLE_CALLJAVA_H
