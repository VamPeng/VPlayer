#include <jni.h>
#include <string>

extern "C" {
#include "libavutil/avutil.h"
}

#include "android/native_window_jni.h"
#include "VFFmpeg.h"
#include "JavaCallHelper.h"
#include "macro.h"

ANativeWindow *window = 0;
VFFmpeg *vfFmpeg;
JavaCallHelper *javaCallHelper;

JavaVM *javaVm = NULL;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVm = vm;
    return JNI_VERSION_1_4;
}

void renderFrame(uint8_t *data_, int linesize, int w, int h) {
    // 渲染

    // 你大爷的这个！！！这个tmd忘了加了调了我一晚上
    ANativeWindow_setBuffersGeometry(window,w,h,WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = 0;
        return;
    }

    uint8_t *window_data = static_cast<uint8_t *>(window_buffer.bits);
    int window_linesize = window_buffer.stride * 4;

    uint8_t *src_data = data_;

    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(window_data + i * window_linesize, src_data + i * linesize, window_linesize);
    }

    ANativeWindow_unlockAndPost(window);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_vam_vplayer_player_VPlayer_native_1prepare(JNIEnv *env, jobject thiz, jstring data_) {

    const char *data = env->GetStringUTFChars(data_, 0);
    javaCallHelper = new JavaCallHelper(javaVm, env, thiz);
    vfFmpeg = new VFFmpeg(javaCallHelper, data);
    vfFmpeg->setRenderCallback(renderFrame);
    vfFmpeg->prepare();
    env->ReleaseStringUTFChars(data_, data);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_vam_vplayer_player_VPlayer_native_1start(JNIEnv *env, jobject thiz) {
    // 正式键入播放状态
    if (vfFmpeg) {
        vfFmpeg->start();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vam_vplayer_player_VPlayer_native_1setSurface(JNIEnv *env, jobject thiz,
                                                       jobject surface_) {
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface_);
}