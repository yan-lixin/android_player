//
// Created by 李鑫 on 2020/11/9.
//

#ifndef FFMPEG_VIDEO_CHANNEL_H
#define FFMPEG_VIDEO_CHANNEL_H

#include "base_channel.h"
#include "audio_channel.h"
#include "macro.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

typedef void (*RenderCallback)(uint8_t *, int, int, int);

class VideoChannel: public BaseChannel {
private:
    pthread_t pid_video_decode;
    pthread_t pid_video_play;
    RenderCallback renderCallback;
    int fps;
    AudioChannel *audioChannel = NULL;
public:
    VideoChannel(int id, AVCodecContext *avCodecContext, int fps, AVRational timeBase, JavaCallHelper *javaCallHelper);

    ~VideoChannel();

    void start();

    void stop();

    void video_decode();

    void video_play();

    void setRenderCallback(RenderCallback renderCallback);

    void setAudioChannel(AudioChannel *audioChannel);
};
#endif //FFMPEG_VIDEO_CHANNEL_H
