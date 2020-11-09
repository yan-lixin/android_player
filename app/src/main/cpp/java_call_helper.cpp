//
// Created by 李鑫 on 2020/11/9.
//

#include "java_call_helper.h"

JavaCallHelper::JavaCallHelper(JavaVM *_javaVm, JNIEnv *_env, jobject _instance) {
    this->javaVm = _javaVm;
    this->env = _env;
    // this->instance = _instance; 不能直接赋值
    // 一旦涉及到jobject 跨方法，跨线程，需要创建全局引用
    this->instance = env->NewGlobalRef(_instance);
    jclass clazz = env->GetObjectClass(this->instance);
    // cd 进入 class所在的目录 执行： javap -s 全限定名,查看输出的 descriptor
    // xx\app\build\intermediates\classes\debug>javap -s com.ffmpeg.sample.Player
    jmd_onPrepared = env->GetMethodID(clazz, "onPrepared", "()V");
    jmd_onError = env->GetMethodID(clazz, "onError", "(I)V");
    jmd_onProgress = env->GetMethodID(clazz, "onProgress", "(I)V");
}

JavaCallHelper::~JavaCallHelper() {
    this->javaVm = NULL;
    this->env->DeleteGlobalRef(this->instance);
    this->instance = NULL;
}

void JavaCallHelper::onPrepared(int threadMode) {
    if (threadMode == THREAD_MAIN) {
        // 主线程
        this->env->CallVoidMethod(this->instance, jmd_onPrepared);
    } else {
        // 子线程
        // 当前子线程的JNIEnv
        JNIEnv *env_child;
        this->javaVm->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(this->instance, jmd_onPrepared);
        this->javaVm->DetachCurrentThread();
    }
}

void JavaCallHelper::onError(int threadMode, int errorCode) {
    if (threadMode == THREAD_MAIN) {
        // 主线程
        this->env->CallVoidMethod(this->instance, jmd_onError);
    } else {
        // 子线程
        JNIEnv *env_child;
        this->javaVm->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(this->instance, jmd_onError, errorCode);
        this->javaVm->DetachCurrentThread();
    }
}

void JavaCallHelper::onProgress(int threadMode, int errorCode) {
    if (threadMode == THREAD_MAIN) {
        // 主线程
        this->env->CallVoidMethod(this->instance, jmd_onProgress);
    } else {
        // 子线程
        JNIEnv *env_child;
        this->javaVm->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(this->instance, jmd_onProgress, errorCode);
        this->javaVm->DetachCurrentThread();
    }
}