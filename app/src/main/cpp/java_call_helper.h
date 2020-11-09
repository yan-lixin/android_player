//
// Created by 李鑫 on 2020/11/9.
//

#ifndef FFMPEG_JAVA_CALL_HELPER_H
#define FFMPEG_JAVA_CALL_HELPER_H

#include <jni.h>
#include "macro.h"

class JavaCallHelper {
private:
    JavaVM *javaVm;
    JNIEnv *env;
    jobject instance;
    jmethodID jmd_onPrepared;
    jmethodID jmd_onError;
    jmethodID jmd_onProgress;
public:
    JavaCallHelper(JavaVM *_javaVm, JNIEnv *_env, jobject _instance);

    ~JavaCallHelper();

    void onPrepared(int threadMode);

    void onError(int threadMode, int errorCode);

    void onProgress(int threadMode, int progress);
};

#endif //FFMPEG_JAVA_CALL_HELPER_H
