
#ifndef MYMUSIC_ANDROIDLOG_H
#define MYMUSIC_ANDROIDLOG_H


#include "android/log.h"


#define LOGD(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG,"liaoweijian",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"liaoweijian",FORMAT,##__VA_ARGS__);
