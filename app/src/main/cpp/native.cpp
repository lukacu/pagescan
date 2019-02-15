#include <jni.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "conversions.h"
#include "logging.h"
#include "detector.h"

using namespace std;
using namespace cv;

static Mat preview, tmp1, tmp2, tmp3;

static Mat capture, document;

extern "C" {

jobject quadrilateral_to_java(JNIEnv *jenv, Quadrilateral q) {

    jclass quadrilateral_class = jenv->FindClass("si/vicos/pagescan/Quadrilateral");

    jmethodID method_quadrilateral_construct = jenv->GetMethodID(quadrilateral_class, "<init>", "(IIIIIIII)V");

    if (!method_quadrilateral_construct) {
        jclass clazz = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(clazz, "Unable to create Quadrilateral class");
        return NULL;
    }

    jobject obj = jenv->NewObject(quadrilateral_class, method_quadrilateral_construct, q.p1.x, q.p1.y, q.p2.x, q.p2.y,
                                  q.p3.x, q.p3.y, q.p4.x, q.p4.y);

    jenv->DeleteLocalRef(quadrilateral_class);

    return obj;



}


JNIEXPORT void JNICALL Java_si_vicos_pagescan_CameraActivity_initializeNative(JNIEnv* jenv, jobject, jobject assetManager) {

}

JNIEXPORT jobject JNICALL Java_si_vicos_pagescan_CameraActivity_processPreviewNative(JNIEnv* jenv, jobject, jobject jimage) {

  if (!convert_image_to_mat(jenv, preview, jimage, IMREAD_GRAYSCALE))
    return NULL;

    Quadrilateral result;

    float scale_factor = (float) preview.cols / 640.0;

    resize(preview, tmp1, Size(640.0, (int)((float)preview.rows / scale_factor)));

    if (get_outline(tmp1, tmp2, tmp3, result)) {

        result.p1 *= scale_factor;
        result.p2 *= scale_factor;
        result.p3 *= scale_factor;
        result.p4 *= scale_factor;

        result.area *= scale_factor;

        return quadrilateral_to_java(jenv, result);

    } else {
        return NULL;
    }

}

JNIEXPORT jboolean JNICALL Java_si_vicos_pagescan_CameraActivity_processCaptureNative(JNIEnv* jenv, jobject, jobject jimage) {

    if (!convert_image_to_mat(jenv, capture, jimage))
        return false;

    Quadrilateral result;

    cvtColor(capture, tmp2, COLOR_BGR2GRAY);

    float scale_factor = (float) capture.cols / 640.0;

    resize(tmp2, tmp1, Size(640.0, (int)((float)capture.rows / scale_factor)));

    if (get_outline(tmp1, tmp2, tmp3, result)) {

        result.p1 *= scale_factor;
        result.p2 *= scale_factor;
        result.p3 *= scale_factor;
        result.p4 *= scale_factor;

        result.area *= scale_factor;

        warp_document(capture, result, document);

        return true;

    } else {

        document.release();

        return false;
    }

}

JNIEXPORT jobject JNICALL Java_si_vicos_pagescan_CameraActivity_retrieveDocumentSize(JNIEnv* jenv, jobject) {

    jclass size_class = jenv->FindClass("android/util/Size");

    jmethodID method_size_construct = jenv->GetMethodID(size_class, "<init>", "(II)V");

    if (!method_size_construct) {
        jclass clazz = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(clazz, "Unable to create Size class");
        return NULL;
    }

    if (document.empty()) {

        jenv->DeleteLocalRef(size_class);
        return NULL;

    } else {

        jobject obj = jenv->NewObject(size_class, method_size_construct, document.cols, document.rows);

        jenv->DeleteLocalRef(size_class);

        return obj;

    }
}

JNIEXPORT jboolean JNICALL Java_si_vicos_pagescan_CameraActivity_retrieveDocumentBitmap(JNIEnv* jenv, jobject, jobject jbitmap) {

    if (document.empty())
        return false;

    return convert_mat_to_bitmap(jenv, jbitmap, document);

}

}
