#include <jni.h>
#include <cstring>
#include <cstdlib>
#include "JniBridge.h"
#include <game-activity/native_app_glue/android_native_app_glue.h>

static int g_exampleIndex = 0;
static int g_sceneIndex = -1;

static jobject g_activityRef = nullptr;

static int g_pendingLabelWidth = 0;
static int g_pendingLabelHeight = 0;
static uint8_t *g_pendingLabelPixels = nullptr;

int getExampleIndex() {
    return g_exampleIndex;
}

int getSceneIndex() {
    return g_sceneIndex;
}

void setPendingBackButtonLabel(int width, int height, const uint8_t *pixels) {
    if (g_pendingLabelPixels) {
        free(g_pendingLabelPixels);
        g_pendingLabelPixels = nullptr;
    }
    g_pendingLabelWidth = width;
    g_pendingLabelHeight = height;
    if (pixels && width > 0 && height > 0) {
        size_t size = static_cast<size_t>(width) * height * 4;
        g_pendingLabelPixels = static_cast<uint8_t *>(malloc(size));
        if (g_pendingLabelPixels)
            memcpy(g_pendingLabelPixels, pixels, size);
    }
}

bool getPendingBackButtonLabel(PendingBackLabel *out) {
    if (!out || !g_pendingLabelPixels) return false;
    out->width = g_pendingLabelWidth;
    out->height = g_pendingLabelHeight;
    out->pixels = g_pendingLabelPixels;
    return true;
}

void clearPendingBackButtonLabel() {
    if (g_pendingLabelPixels) {
        free(g_pendingLabelPixels);
        g_pendingLabelPixels = nullptr;
    }
    g_pendingLabelWidth = 0;
    g_pendingLabelHeight = 0;
}

void requestFinishActivity(android_app *app) {
    if (!app || !app->activity || !g_activityRef) return;
    JavaVM *vm = app->activity->vm;
    if (!vm) return;
    JNIEnv *env = nullptr;
    jint res = vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (res == JNI_EDETACHED)
        res = vm->AttachCurrentThread(&env, nullptr);
    if (res != JNI_OK || !env) return;
    jclass clazz = env->GetObjectClass(g_activityRef);
    if (!clazz) return;
    jmethodID mid = env->GetMethodID(clazz, "scheduleFinish", "()V");
    if (mid)
        env->CallVoidMethod(g_activityRef, mid);
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_genesisv_MainActivity_setExampleIndex(JNIEnv *env, jobject thiz, jint index) {
    if (g_activityRef && env)
        env->DeleteGlobalRef(g_activityRef);
    g_activityRef = (thiz && env) ? env->NewGlobalRef(thiz) : nullptr;
    g_exampleIndex = static_cast<int>(index);
    g_sceneIndex = -1;
}

JNIEXPORT void JNICALL
Java_com_example_genesisv_MainActivity_setSceneIndex(JNIEnv *env, jobject thiz, jint index) {
    if (g_activityRef && env)
        env->DeleteGlobalRef(g_activityRef);
    g_activityRef = (thiz && env) ? env->NewGlobalRef(thiz) : nullptr;
    g_sceneIndex = static_cast<int>(index);
    g_exampleIndex = 0;
}

JNIEXPORT void JNICALL
Java_com_example_genesisv_MainActivity_setBackButtonLabelBitmap(JNIEnv *env, jobject thiz,
                                                                  jint width, jint height,
                                                                  jbyteArray pixels) {
    (void) thiz;
    if (!pixels || width <= 0 || height <= 0) return;
    jsize len = env->GetArrayLength(pixels);
    if (len < width * height * 4) return;
    jbyte *data = env->GetByteArrayElements(pixels, nullptr);
    if (data) {
        setPendingBackButtonLabel(width, height, reinterpret_cast<const uint8_t *>(data));
        env->ReleaseByteArrayElements(pixels, data, JNI_ABORT);
    }
}

}
