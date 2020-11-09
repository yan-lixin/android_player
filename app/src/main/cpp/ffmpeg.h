//
// Created by 李鑫 on 2020/11/9.
//

#ifndef FFMPEG_FFMPEG_H
#define FFMPEG_FFMPEG_H

#include "java_call_helper.h"
#include "audio_channel.h"
#include "video_channel.h"
#include "macro.h"
#include <cstring>
#include <pthread.h>

extern "C" {
#include <libavformat/avformat.h>
}

class FFmpeg {
    friend void *task_stop(void *args);

private:
    JavaCallHelper *javaCallHelper = NULL;
    AudioChannel *audioChannel = NULL;
    VideoChannel *videoChannel = NULL;
    char *dataSource;
    pthread_t pid_prepare;
    pthread_t pid_start;
    pthread_t pid_stop;
    bool isPlaying;
    AVFormatContext *formatContext = NULL;
    RenderCallback renderCallback;
    int duration;
    pthread_mutex_t seekMutex;

public:
    FFmpeg(JavaCallHelper *javaCallHelper, char *dataSource);

    ~FFmpeg();

    void prepare();

    void _prepare();

    void start();

    void _start();

    void setRenderCallback(RenderCallback renderCallback);

    void stop();

    void setDuration(int duration);

    //总播放时长
    int getDuration() const;

    void seekTo(int i);
};

#endif //FFMPEG_FFMPEG_H
