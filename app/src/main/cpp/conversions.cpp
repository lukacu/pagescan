
#include <android/bitmap.h>
#include <opencv2/imgproc.hpp>
#include <android/log.h>

#include "conversions.h"
#include "logging.h"

using namespace std;
using namespace cv;

inline void yuv2bgr(uchar y, uchar u, uchar v, uchar *out) {
    int rTmp = y + (1.370705 * (v-128));
    int gTmp = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
    int bTmp = y + (1.732446 * (u-128));
    out[2] = saturate_cast<uchar>(rTmp);
    out[1] = saturate_cast<uchar>(gTmp);
    out[0] = saturate_cast<uchar>(bTmp);
}

bool convert_image_to_mat(JNIEnv *jenv, cv::Mat& image, jobject jimage, int flags) {

    jclass img_cls = jenv->FindClass("android/media/Image");

    //jclass in_cls = (*env)->GetObjectClass(jimage);

    if (!jenv-> IsInstanceOf(jimage, img_cls)) {
        jclass clazz = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(clazz, "Unable to convert image");
        return false;
    }

    jmethodID method_get_width = jenv->GetMethodID(img_cls, "getWidth", "()I");
    jmethodID method_get_height = jenv->GetMethodID(img_cls, "getHeight", "()I");
    jmethodID method_get_format = jenv->GetMethodID(img_cls, "getFormat", "()I");

    jclass plane_cls = jenv->FindClass("android/media/Image$Plane");

    jmethodID method_get_planes = jenv->GetMethodID(img_cls, "getPlanes", "()[Landroid/media/Image$Plane;");

    jmethodID method_get_buffer = jenv->GetMethodID(plane_cls, "getBuffer", "()Ljava/nio/ByteBuffer;");
    jmethodID method_get_row_stride = jenv->GetMethodID(plane_cls, "getRowStride", "()I");
    jmethodID method_get_pixel_stride = jenv->GetMethodID(plane_cls, "getPixelStride", "()I");

    jint image_type = jenv->CallIntMethod(jimage, method_get_format);
    jint image_width = jenv->CallIntMethod(jimage, method_get_width);
    jint image_height = jenv->CallIntMethod(jimage, method_get_height);

    jobjectArray planes = (jobjectArray) jenv->CallObjectMethod(jimage, method_get_planes);

    switch (image_type) {
        case JAVA_BUFFER_TYPE_YUV_420_888: {

            jobject y_plane = jenv->GetObjectArrayElement(planes, 0);
            jobject u_plane = jenv->GetObjectArrayElement(planes, 1);
            jobject v_plane = jenv->GetObjectArrayElement(planes, 2);

            jobject  y_buffer = jenv->CallObjectMethod(y_plane, method_get_buffer);
            jobject  u_buffer = jenv->CallObjectMethod(u_plane, method_get_buffer);
            jobject  v_buffer = jenv->CallObjectMethod(v_plane, method_get_buffer);

            jint u_stride = jenv->CallIntMethod(u_plane, method_get_row_stride);
            jint v_stride = jenv->CallIntMethod(v_plane, method_get_row_stride);

            jint u_pixel = jenv->CallIntMethod(u_plane, method_get_pixel_stride);
            jint v_pixel = jenv->CallIntMethod(v_plane, method_get_pixel_stride);

            uchar* y_p = (uchar *) jenv->GetDirectBufferAddress(y_buffer);
            uchar* u_p = (uchar *) jenv->GetDirectBufferAddress(u_buffer);
            uchar* v_p = (uchar *) jenv->GetDirectBufferAddress(v_buffer);

            if (flags == cv::IMREAD_GRAYSCALE) {
                image.create(cv::Size(image_width, image_height), CV_8UC1);
                uchar* dest = image.ptr<uchar>(0);
                memcpy(dest, y_p, image_height * image_width * sizeof(uchar));
                return true;
            }

            image.create(cv::Size(image_width, image_height), CV_8UC3);

            uchar y, cb, cr;
            uchar r, g, b;

            uchar* dest1 = image.ptr<uchar>(0);
            uchar* dest2 = dest1 + 3 * image_width;

            uchar* u_p1, *v_p1, *y_p1;

            for (int j = 0; j < image_height / 2; j++) {

                u_p1 = u_p;
                v_p1 = v_p;
                y_p1 = y_p + image_width;

                for (int i = 0; i < image_width / 2; i++) {

                    cb = u_p1[0];
                    cr = v_p1[0];

                    yuv2bgr(y_p[0], cb, cr, dest1);
                    dest1 += 3;
                    yuv2bgr(y_p[1], cb, cr, dest1);
                    dest1 += 3;
                    yuv2bgr(y_p1[0], cb, cr, dest2);
                    dest2 += 3;
                    yuv2bgr(y_p1[1], cb, cr, dest2);
                    dest2 += 3;

                    y_p += 2;
                    y_p1 += 2;
                    u_p1 += u_pixel;
                    v_p1 += v_pixel;
                }

                dest1 += 3 * image_width;
                dest2 += 3 * image_width;
                u_p += u_stride;
                v_p += v_stride;
                y_p += image_width;
            }

            return true;
        }
        case JAVA_BUFFER_TYPE_JPEG: {
            jobject data_plane = jenv->GetObjectArrayElement(planes, 0);
            jobject  data_buffer = jenv->CallObjectMethod(data_plane, method_get_buffer);

            jsize l = jenv->GetDirectBufferCapacity(data_buffer);
            void* a = jenv->GetDirectBufferAddress(data_buffer);

            Mat buffer(1, l, CV_8UC1, a);
            image = imdecode(buffer, flags, &image);
            return true;

        }
    }

    jclass clazz = jenv->FindClass("java/lang/Exception");
    jenv->ThrowNew(clazz, "Unable to convert image");
    return false;

}

bool convert_mat_to_bitmap(JNIEnv *jenv, jobject jbitmap, Mat image) {

  AndroidBitmapInfo info;
  void* a;

  if (AndroidBitmap_getInfo(jenv, jbitmap, &info) != ANDROID_BITMAP_RESULT_SUCCESS) {
    LOGW("Unable to get bitmap header");
    return false;
  }

  if (image.cols != (int)info.width || image.rows != (int) info.height) {
    LOGW("Bitmap is not the same size as source image (%dx%d) vs. (%dx%d)", info.width, info.height, image.cols, image.rows);
    return false;
  }

  if (image.channels() != 1 && image.channels() != 3) {
    LOGW("Source image must contain one or three channels");
    return false;  
  }

  bool color = image.channels() != 1;

  if (AndroidBitmap_lockPixels(jenv, jbitmap, &a) != ANDROID_BITMAP_RESULT_SUCCESS)
    return false;

  switch (info.format) {
    case ANDROID_BITMAP_FORMAT_RGBA_8888: {
      Mat rgba(info.height, info.width, CV_8UC4, (uchar*)a);
      if (color)
        cvtColor(image, rgba, COLOR_BGR2RGBA);
      else 
        cvtColor(image, rgba, COLOR_GRAY2RGBA);
      break;
    } 
    case ANDROID_BITMAP_FORMAT_A_8: {
      Mat gray(info.height, info.width, CV_8UC1, (uchar*)a);
      if (color)
        cvtColor(image, gray, COLOR_BGR2GRAY);
      else 
        image.copyTo(gray);
      break;
    }
  }

  AndroidBitmap_unlockPixels(jenv, jbitmap);

  return true;
}

jobject rectangle_to_java(JNIEnv *jenv, Rect rectangle) {

    jclass rectangle_class = jenv->FindClass("si/vicos/Rectangle");

    jmethodID method_rectangle_construct = jenv->GetMethodID(rectangle_class, "<init>", "(IIII)V");

    if (!method_rectangle_construct) {
        jclass clazz = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(clazz, "Unable to create Rectangle class");
        return NULL;
    }
  
    jobject obj = jenv->NewObject(rectangle_class, method_rectangle_construct, rectangle.x, rectangle.y, rectangle.x + rectangle.width, rectangle.y + rectangle.height);

    jenv->DeleteLocalRef(rectangle_class);

    return obj;

}

jobject line_to_java(JNIEnv *jenv, cv::Vec4i line) {

    jclass line_class = jenv->FindClass("si/vicos/Line");

    jmethodID method_line_construct = jenv->GetMethodID(line_class, "<init>", "(IIII)V");

    if (!method_line_construct) {
        jclass clazz = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(clazz, "Unable to create Line class");
        return NULL;
    }

    jobject obj = jenv->NewObject(line_class, method_line_construct, line[0], line[1], line[2], line[3]);

    jenv->DeleteLocalRef(line_class);

    return obj;

}