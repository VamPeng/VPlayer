//
// Created by Yuhui Peng on 2023/3/7.
//

#include "VideoChannel.h"
#include "macro.h"

extern "C" {
#include "libavutil/time.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}


void dropPacket(queue<AVPacket *> &q) {
    while (!q.empty()) {
        AVPacket *pkt = q.front();
        if (pkt->flags != AV_PKT_FLAG_KEY) {
            q.pop();
            BaseChannel::releaseAVPacket(pkt);
        } else {
            break;
        }
    }
}

void dropFrame(queue<AVFrame *> &q) {
    if (!q.empty()){
        AVFrame *frame = q.front();
        q.pop();
        BaseChannel::releaseAVFrame(frame);
    }
}

VideoChannel::VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext,
                           AVRational avRational_)
        : BaseChannel(id, javaCallHelper, codecContext, avRational_) {
    frame_queue.setReleaseCallback(releaseAVFrame);
    frame_queue.setSyncHandle(dropFrame);
}

void *decode(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decodePacket();
    return 0;
}

void *synchronize(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->synchronizeFrame();
    return 0;
}

void VideoChannel::play() {
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;

    pthread_create(&pid_video_play, NULL, decode, this);
    pthread_create(&pid_synchronize, NULL, synchronize, this);

}

void VideoChannel::decodePacket() {
    AVPacket *avPacket = 0;
    while (isPlaying) {
        int ret = pkt_queue.get(avPacket);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }

        if (!avPacket) {
            continue;
        }

        if (!avCodecContext) {
            continue;
        }

        ret = avcodec_send_packet(avCodecContext, avPacket);

        releaseAVPacket(avPacket);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            // 失败
            break;
        }

        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);

        // 压缩数据 要解压
        frame_queue.push(frame);
//        LOGE("frame添加成功%d",frame_queue.size());
        while (frame_queue.size() > 100 && isPlaying) {
            av_usleep(1000 * 10);
            continue;
        }
    }
    releaseAVPacket(avPacket);
}

void VideoChannel::stop() {

}

void VideoChannel::synchronizeFrame() {
    //初始化 转换器上下文
    SwsContext *sws_ctx = sws_getContext(
            avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,
            avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, 0, 0, 0
    );
    uint8_t *dst_data[4]; // argb
    int dst_linesize[4];

    av_image_alloc(dst_data, dst_linesize,
                   avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA, 1);
    AVFrame *frame = 0;
    while (isPlaying) {
        int ret = frame_queue.get(frame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }

        if (!frame) {
            continue;
        }

        sws_scale(sws_ctx,
                  frame->data,
                  frame->linesize, 0,
                  frame->height,
                  dst_data,
                  dst_linesize);
        renderFrame(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);

        clock = (double) frame->pts * av_q2d(time_base);
        double frame_delays = 1.0 / fps;

        double audioClock = audioChannel->clock;
        double diff = clock - audioClock;

        // 解码时间也要算进去，配置差的手机解码耗时也多一些
        double extra_delay = frame->repeat_pict / (2 * fps);
        double delay = extra_delay + frame_delays;

        if (clock > audioClock) {
            if (diff > 1) {
                av_usleep((delay * 2) * 1000000);
            } else {
                av_usleep((delay + diff) * 1000000);
            }
        } else {
            if (diff > 1) {
                // 不休眠
            } else if (diff >= 0.05) {
                // 视频需要追赶 丢非关键帧
                releaseAVFrame(frame);
                frame_queue.sync();
            } else {

            }
        }


        releaseAVFrame(frame);
    }
    av_freep(&dst_data[0]);
    isPlaying = false;
    releaseAVFrame(frame);
    sws_freeContext(sws_ctx);
}

void VideoChannel::setRenderFrame(RenderFrame renderFrame1) {
    this->renderFrame = renderFrame1;

}

void VideoChannel::setFps(double fps_) {
    fps = fps_;
}

