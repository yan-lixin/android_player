//
// Created by 李鑫 on 2020/11/9.
//

#include "video_channel.h"

/**
 * 丢包 （AVPacket）
 * @param q
 */
void dropAVPacket(queue<AVPacket *> &q) {
    while (!q.empty()) {
        AVPacket *avPacket = q.front();
        if (avPacket->flags != AV_PKT_FLAG_KEY) {
            BaseChannel::releaseAVPacket(&avPacket);
            q.pop();
        } else {
            break;
        }
    }
}

/**
 * 丢包 （AVFrame）
 * @param q
 */
void dropAVFrame(queue<AVFrame *> &q) {
    while (!q.empty()) {
        AVFrame *avFrame = q.front();
        BaseChannel::releaseAVFrame(&avFrame);
        q.pop();
    }
}

VideoChannel::VideoChannel(int id, AVCodecContext *avCodecContext, int fps, AVRational timeBase,
                           JavaCallHelper *javaCallHelper): BaseChannel(id, avCodecContext, timeBase, javaCallHelper) {
    this->fps = fps;
    packets.setSyncHandle(dropAVPacket);
    frames.setSyncHandle(dropAVFrame);
}

VideoChannel::~VideoChannel() {}

void *task_video_decode(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_decode();
    return 0;
}

void *task_video_play(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_play();
    return 0;
}

void VideoChannel::start() {
    this->isPlaying = 1;
    // 设置队列为工作状态
    this->packets.setWork(1);
    this->frames.setWork(1);
    // 解码
    pthread_create(&pid_video_decode, 0, task_video_decode, this);
    // 播放
    pthread_create(&pid_video_play, 0, task_video_play, this);
}

/**
 * 视频解码
 */
void VideoChannel::video_decode() {
    AVPacket *packet = NULL;
    while (isPlaying) {
        int ret = packets.pop(packet);
        if (!isPlaying) {
            // 如果停止播放，跳出循环，释放packet
            break;
        }
        if (!ret) {
            // 读取数据包失败
            continue;
        }
        // 得到了视频数据包，需要把数据包给解码器进行解码
        ret = avcodec_send_packet(codecContext, packet);
        if (ret) {
            // 往解码器发送数据包失败，跳转循环
            break;
        }
        // 释放Packet
        releaseAVPacket(&packet);
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            // 重新来
            continue;
        } else if (ret != 0) {
            break;
        }
        // ret == 0表示数据收发正常，成功获取到了解码后的视频原始数据包 AVFrame，格式是YUV
        while (isPlaying && frames.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        frames.push(frame);
    }
    releaseAVPacket(&packet);
}

void VideoChannel::video_play() {
    AVFrame *frame = NULL;
    // 对原始数据进行格式转换 YUV->RGBA
    uint8_t  *dst_data[4];
    int dst_line_size[4];
    SwsContext *swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt, codecContext-> width, codecContext->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);

    //  给dst_data dst_linesize申请内存
    av_image_alloc(dst_data, dst_line_size, codecContext->width, codecContext->height, AV_PIX_FMT_RGBA, 1);
    // 根据fps（传入的流的平均帧率来控制每一帧的延时时间）
    // sleep: fps / 时间
    // 单位是秒
    double delay_time_per_frame = 1.0 / fps;
    while (isPlaying) {
        int ret = frames.pop(frame);
        if (!isPlaying) {
            // 如果停止播放了，跳出循环 释放packet
            break;
        }
        if (!ret) {
            // 取数据包失败
            continue;
        }
        // 取到了YUV原始数据，进行格式转换
        sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height, dst_data, dst_line_size);
        // 进行休眠
        // 每一帧还有自己的额外延时时间
        // extra_delay = repeat_pict / (2*fps)
        double extra_delay = frame->repeat_pict / (2 * fps);
        double real_delay = delay_time_per_frame + extra_delay;
        // 单位是微妙
        // av_usleep(real_delay * 10000000); // 直接以视频播放规则来播放

        // 需要根据音频的播放时间来判断
        // 获取视频的播放时间
        double videoTime = frame->best_effort_timestamp * av_q2d(timeBase);

        // 音视频同步
        if (!audioChannel) {
            // 没有音频
            av_usleep(real_delay * 1000000);
            if (javaCallHelper) {
                javaCallHelper->onProgress(THREAD_CHILD, videoTime);
            }
        } else {
            double audioTime = audioChannel->audioTime;
            // 获取音视频播放的时间差
            double timeDiff = videoTime - audioTime;
            if (timeDiff > 0) {
                LOGI("视频比音频快：%lf", timeDiff);
                // 视频比音频快：等音频（sleep）
                // 自然播放状态下，time_diff的值不会很大
                // 但是，seek后time_diff的值可能会很大，导致视频休眠太久

                // av_usleep((real_delay + time_diff) * 1000000);//TODO seek后测试

                if (timeDiff > 1) {
                    // 等音频慢慢赶上来
                    av_usleep((real_delay * 2) * 1000000);
                } else {
                    av_usleep((real_delay + timeDiff) * 1000000);
                }
            } else if(timeDiff < 0) {
                LOGI("音频比视频快：%lf", fabs(timeDiff));
                // 音频比视频快：追音频（尝试丢视频包）
                // 视频包：packets和frames
                if (fabs(timeDiff) >= 0.05) {
                    // 时间差如果大于0.05，有明显的延迟感
                    // 丢包：要操作队列中数据！一定要小心！
                    // packets.sync();
                    frames.sync();
                    continue;
                }
            } else {
                LOGI("音视频完美同步");
            }
        }

        // dst_data: AV_PIX_FMT_RGBA格式的数据
        // 渲染，回调出去> native-lib里
        // 渲染一副图像 需要什么信息？
        // 宽高！> 图像的尺寸！
        // 图像的内容！(数据)>图像怎么画
        // 需要：1，data;2，linesize；3，width; 4， height
        renderCallback(dst_data[0], dst_line_size[0], codecContext->width, codecContext->height);
        releaseAVFrame(&frame);
    }
    releaseAVFrame(&frame);
    isPlaying = 0;
    av_freep(&dst_data[0]);
    sws_freeContext(swsContext);
}

void VideoChannel::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel = audioChannel;
}

void VideoChannel::stop() {
    isPlaying = 0;
    javaCallHelper = NULL;
    packets.setWork(0);
    frames.setWork(0);
    pthread_join(pid_video_decode, 0);
    pthread_join(pid_video_play, 0);
}