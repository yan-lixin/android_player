//
// Created by 李鑫 on 2020/11/9.
//

#ifndef FFMPEG_BASE_CHANNEL_H
#define FFMPEG_BASE_CHANNEL_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
}

#include "safe_queue.h"
#include "java_call_helper.h"

class BaseChannel {
public:
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    int id;
    bool isPlaying = 0;
    // 解码器上下文
    AVCodecContext *codecContext;
    AVRational timeBase;
    double audioTime;
    JavaCallHelper *javaCallHelper = NULL;
public:
    BaseChannel(int id, AVCodecContext *codecContext, AVRational timeBase, JavaCallHelper *javaCallHelper): id(id), codecContext(codecContext), timeBase(timeBase), javaCallHelper(javaCallHelper) {
        packets.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
    }

    virtual ~BaseChannel() {
        packets.clear();
        frames.clear();
        if (codecContext) {
            avcodec_close(codecContext);
            avcodec_free_context(&codecContext);
            codecContext = NULL;
        }
    }

    /**
     * 释放AVPacket
     * @param packet
     */
    static void releaseAVPacket(AVPacket **packet) {
        if (packet) {
            av_packet_free(packet);
            *packet = NULL;
        }
    }

    static void releaseAVFrame(AVFrame **frame) {
        if (frame) {
            av_frame_free(frame);
            *frame = NULL;
        }
    }

    virtual void start() = 0;

    virtual void stop() = 0;
};

#endif //FFMPEG_BASE_CHANNEL_H
