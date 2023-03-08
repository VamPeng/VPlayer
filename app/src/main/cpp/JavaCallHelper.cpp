//
// Created by Yuhui Peng on 2023/3/7.
//

#include "JavaCallHelper.h"
#include "macro.h"

JavaCallHelper::JavaCallHelper(JavaVM *_javaVM, JNIEnv *_env, jobject &_jobj) :
        javaVM(_javaVM), jniEnv(_env) {

    jobj = _env->NewGlobalRef(_jobj);
    jclass jclzz = jniEnv->GetObjectClass(jobj);

    jmid_error = jniEnv->GetMethodID(jclzz, "onError", "(I)V");
    jmid_prepare = jniEnv->GetMethodID(jclzz, "onPrepared", "()V");
    jmid_progress = jniEnv->GetMethodID(jclzz, "onProgress", "(I)V");
}

void JavaCallHelper::onError(int thread, int code) {
    if (thread == THREAD_CHILD) {

        JNIEnv *_jniEnv;
        if (javaVM->AttachCurrentThread(&_jniEnv, 0) != JNI_OK) {
            return;
        }
        _jniEnv->CallVoidMethod(jobj, jmid_error, code);
        javaVM->DetachCurrentThread();
    } else {
        jniEnv->CallVoidMethod(jobj, jmid_error, code);
    }
}

void JavaCallHelper::onPrepare(int thread) {
    if (thread == THREAD_CHILD) {

        JNIEnv *_jniEnv;
        if (javaVM->AttachCurrentThread(&_jniEnv, 0) != JNI_OK) {
            return;
        }
        _jniEnv->CallVoidMethod(jobj, jmid_prepare);
        javaVM->DetachCurrentThread();
    } else {
        jniEnv->CallVoidMethod(jobj, jmid_prepare);
    }
}

void JavaCallHelper::onProgress(int thread, int progress) {
    if (thread == THREAD_CHILD) {

        JNIEnv *_jniEnv;
        if (javaVM->AttachCurrentThread(&_jniEnv, 0) != JNI_OK) {
            return;
        }
        _jniEnv->CallVoidMethod(jobj, jmid_progress, progress);
        javaVM->DetachCurrentThread();
    } else {
        jniEnv->CallVoidMethod(jobj, jmid_progress, progress);
    }
}

JavaCallHelper::~JavaCallHelper() {

}
