#ifndef _LOGGING_H
#define _LOGGING_H

#define  LOG_TAG    "PageScan"

#ifdef __ANDROID__

#include <android/log.h>

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#else

#include <cstdio>

#define  LOGD(...)  { printf(__VA_ARGS__); printf("\n"); }
#define  LOGI(...)  { printf(__VA_ARGS__); printf("\n"); }
#define  LOGW(...)  { printf(__VA_ARGS__); printf("\n"); }
#define  LOGE(...)  { printf(__VA_ARGS__); printf("\n"); }

#endif

#endif
