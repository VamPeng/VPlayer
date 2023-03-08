//
// Created by Yuhui Peng on 2023/3/7.
//

#ifndef VPLAYER_BASECHANNEL_H
#define VPLAYER_BASECHANNEL_H

#include "safe_queue.h"
#include "JavaCallHelper.h"
#include "pthread.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

class BaseChannel {
public:
    BaseChannel(int id, JavaCallHelper *_javaCallHelper, AVCodecContext *_codecContext,
                AVRational time_base)
            : channelId(id), javaCallHelper(_javaCallHelper), avCodecContext(_codecContext),
              time_base(time_base) {

    }

    ~BaseChannel() {
        if (avCodecContext) {
            avcodec_close(avCodecContext);
            avcodec_free_context(&avCodecContext);
            avCodecContext = 0;
        }
        pkt_queue.clear();
        frame_queue.clear();
    }

    virtual void play() = 0;

    virtual void stop() = 0;

    static void releaseAVPacket(AVPacket *&packet) {
        if (packet) {
            av_packet_free(&packet);
            packet = 0;
        }
    }

    static void releaseAVFrame(AVFrame *&frame) {
        if (frame) {
            av_frame_free(&frame);
            frame = 0;
        }
    }
    SafeQueue2<AVPacket *> pkt_queue;

    SafeQueue2<AVFrame *> frame_queue;
    volatile int channelId;
    volatile bool isPlaying;
    AVCodecContext *avCodecContext;

    JavaCallHelper *javaCallHelper;

    AVRational time_base;

    double clock = 0;

};

#endif //VPLAYER_BASECHANNEL_H
