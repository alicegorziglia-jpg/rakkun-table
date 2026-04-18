#include <jni.h>
#include <android/native_window.h>
#include <android/log.h>

#include "video_decoder.h"
#include "renderer.h"
#include "input_capture.h"
#include "network_client.h"

#define LOG_TAG "PenStreamNDK"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static penstream::android::VideoDecoder* g_decoder = nullptr;
static penstream::android::Renderer* g_renderer = nullptr;
static penstream::android::NetworkClient* g_client = nullptr;

extern "C" {

JNIEXPORT void JNICALL
Java_com_penstream_app_PenStreamService_nativeInit(JNIEnv* env, jobject thiz) {
    LOGI("Native init");

    g_decoder = new penstream::android::VideoDecoder();
    g_renderer = new penstream::android::Renderer();
    g_client = new penstream::android::NetworkClient();

    g_client->set_frame_callback([](const std::vector<uint8_t>& data) {
        if (g_decoder) {
            g_decoder->decode(data);
        }
    });
}

JNIEXPORT void JNICALL
Java_com_penstream_app_PenStreamService_nativeStartStreaming(JNIEnv* env, jobject thiz,
                                                              jstring address, jint port) {
    LOGI("Starting streaming");

    const char* addr = env->GetStringUTFChars(address, nullptr);
    int p = static_cast<int>(port);

    if (g_client) {
        g_client->connect(addr, p);
        g_client->start();
    }

    env->ReleaseStringUTFChars(address, addr);
}

JNIEXPORT void JNICALL
Java_com_penstream_app_PenStreamService_nativeStopStreaming(JNIEnv* env, jobject thiz) {
    LOGI("Stopping streaming");

    if (g_client) {
        g_client->stop();
    }
}

JNIEXPORT void JNICALL
Java_com_penstream_app_PenStreamService_nativeSendInput(JNIEnv* env, jobject thiz,
                                                         jfloat x, jfloat y, jfloat pressure,
                                                         jint tilt_x, jint tilt_y, jint buttons) {
    if (g_client) {
        g_client->send_input(x, y, pressure,
                             static_cast<int8_t>(tilt_x),
                             static_cast<int8_t>(tilt_y),
                             static_cast<uint8_t>(buttons));
    }
}

JNIEXPORT void JNICALL
Java_com_penstream_app_PenStreamService_nativeSetSurface(JNIEnv* env, jobject thiz,
                                                          jobject surface) {
    LOGI("Setting surface");

    ANativeWindow* window = nullptr;
    if (surface != nullptr) {
        window = ANativeWindow_fromSurface(env, surface);
    }

    if (g_renderer) {
        g_renderer->set_surface(window);
    }

    if (window != nullptr) {
        ANativeWindow_release(window);
    }
}

JNIEXPORT void JNICALL
Java_com_penstream_app_PenStreamService_nativeRelease(JNIEnv* env, jobject thiz) {
    LOGI("Native release");

    delete g_decoder;
    delete g_renderer;
    delete g_client;

    g_decoder = nullptr;
    g_renderer = nullptr;
    g_client = nullptr;
}

} // extern "C"
