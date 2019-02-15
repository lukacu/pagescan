#ifndef _ANDROID_CONVERSIONS_H
#define _ANDROID_CONVERSIONS_H

#include <jni.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#define JAVA_BUFFER_TYPE_CUSTOM 0
#define JAVA_BUFFER_TYPE_INT_RGB 1
#define JAVA_BUFFER_TYPE_INT_ARGB 2
#define JAVA_BUFFER_TYPE_INT_ARGB_PRE 3
#define JAVA_BUFFER_TYPE_INT_BGR 4
#define JAVA_BUFFER_TYPE_3BYTE_BGR 5
#define JAVA_BUFFER_TYPE_4BYTE_ABGR 6
#define JAVA_BUFFER_TYPE_4BYTE_ABGR_PRE 7
#define JAVA_BUFFER_TYPE_USHORT_565_RGB 8
#define JAVA_BUFFER_TYPE_USHORT_555_RGB 9
#define JAVA_BUFFER_TYPE_BYTE_GRAY 10
#define JAVA_BUFFER_TYPE_USHORT_GRAY 11
#define JAVA_BUFFER_TYPE_BYTE_BINARY 12
#define JAVA_BUFFER_TYPE_JPEG 256
#define JAVA_BUFFER_TYPE_NV21 17
#define JAVA_BUFFER_TYPE_YUV_420_888 35
#define JAVA_BUFFER_TYPE_RGB656 1004
#define JAVA_BUFFER_TYPE_YUY2 1020

bool convert_image_to_mat(JNIEnv *jenv, cv::Mat& image, jobject jimage, int flags = cv::IMREAD_UNCHANGED);

bool convert_mat_to_bitmap(JNIEnv *jenv, jobject jbitmap, cv::Mat image);

jobject rectangle_to_java(JNIEnv *jenv, cv::Rect rectangle);

jobject line_to_java(JNIEnv *jenv, cv::Vec4i line);

template<typename T>
jobject objects_to_java(JNIEnv *jenv, std::vector<T> list, jobject (*converter)(JNIEnv *, T)) {

    jclass vector_class = jenv->FindClass("java/util/ArrayList");

    jmethodID method_vector_construct = jenv->GetMethodID(vector_class, "<init>", "()V");
    jmethodID method_vector_add = jenv->GetMethodID(vector_class, "add", "(Ljava/lang/Object;)Z");

    if (!method_vector_construct || !method_vector_add) {
        jclass clazz = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(clazz, "Unable to create Java ArrayList class");
        return NULL;
    }

    jobject jlist = jenv->NewObject(vector_class, method_vector_construct);

    typedef typename std::vector<T>::iterator iterator;

    for (iterator it = list.begin(); it != list.end(); it++) {
        jenv->CallBooleanMethod(jlist, method_vector_add, converter(jenv, *it));
    }

    jenv->DeleteLocalRef(vector_class);

    return jlist;
}

#endif
