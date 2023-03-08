//
// Created by Yuhui Peng on 2023/3/7.
//

#include "VFFmpeg.h"
#include "macro.h"

VFFmpeg::VFFmpeg(JavaCallHelper *javaCallHelper_, const char *dataSource) {
    url = new char[strlen(dataSource) + 1];
    strcpy(url, dataSource);
    javaCallHelper = javaCallHelper_;
}

void *prepareFFmpeg_(void *args) {
    VFFmpeg *vfFmpeg = static_cast<VFFmpeg *>(args);
    vfFmpeg->prepareFFmpeg();
    return 0;
}

void *startThread(void *args) {
    VFFmpeg *vfFmpeg = static_cast<VFFmpeg *>(args);
    vfFmpeg->play();
    return 0;
}

VFFmpeg::~VFFmpeg() {

}

void VFFmpeg::prepare() {
// 耗时操作，线程使用
    pthread_create(&pid_prepare, NULL, prepareFFmpeg_, this);

}

void VFFmpeg::prepareFFmpeg() {
    avformat_network_init();

    formatContext = avformat_alloc_context();

    AVDictionary *opts = NULL;

    av_dict_set(&opts, "timeout", "3000000", 0);

    int ret = avformat_open_input(&formatContext, url, NULL, &opts);

    if (ret != 0) {
        childThreadError(FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        childThreadError(FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVCodecParameters *codecpar = formatContext->streams[i]->codecpar;
        AVStream *stream = formatContext->streams[i];

        const AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);

        if (!dec) {
            childThreadError(FFMPEG_FIND_DECODER_FAIL);
            return;
        }

        AVCodecContext *codecContext = avcodec_alloc_context3(dec);
        if (!codecContext) {
            childThreadError(FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }

        if (avcodec_parameters_to_context(codecContext, codecpar) < 0) {
            childThreadError(FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }

        if (avcodec_open2(codecContext, dec, 0) != 0) {
            childThreadError(FFMPEG_OPEN_DECODER_FAIL);
            return;
        }

        AVRational time_base = stream->time_base;

        if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel(i, javaCallHelper, codecContext, time_base);
        } else if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoChannel = new VideoChannel(i, javaCallHelper, codecContext, time_base);
            videoChannel->setRenderFrame(renderFrame);
            videoChannel->setFps(av_q2d(stream->avg_frame_rate));
        }


    }

    if (!audioChannel && !videoChannel) {
        childThreadError(FFMPEG_NO_MEDIA);
        return;
    }

    videoChannel->audioChannel = audioChannel;

    if (javaCallHelper) {
        javaCallHelper->onPrepare(THREAD_CHILD);
    }

}

void VFFmpeg::childThreadError(int code) {
    if (javaCallHelper) {
        javaCallHelper->onError(THREAD_CHILD, code);
    }
}

void VFFmpeg::start() {
    isPlaying = true;
    if (audioChannel) {
        audioChannel->play();
    }
    if (videoChannel) {
        videoChannel->play();
    }
    pthread_create(&pid_play, NULL, startThread, this);
}

void VFFmpeg::play() {
    int ret = 0;
    while (isPlaying) {
        if (audioChannel && audioChannel->pkt_queue.size() > 100) {
            // packet队列生成过快
            // 休眠10ms
            av_usleep(1000 * 10);
            continue;
        }

        if (videoChannel && videoChannel->pkt_queue.size() > 100) {
            // packet队列生成过快
            // 休眠10ms
            av_usleep(1000 * 10);
            continue;
        }

        AVPacket *packet = av_packet_alloc();
        ret = av_read_frame(formatContext, packet);

        if (ret == 0) {
            if (audioChannel && packet->stream_index == audioChannel->channelId) {
                audioChannel->pkt_queue.push(packet);
            } else if (videoChannel && packet->stream_index == videoChannel->channelId) {
                videoChannel->pkt_queue.push(packet);
            }
        } else if (ret == AVERROR_EOF) {
            if (videoChannel && videoChannel->pkt_queue.empty() && videoChannel->frame_queue.empty()
                && audioChannel && audioChannel->pkt_queue.empty() &&
                audioChannel->frame_queue.empty()) {
                LOGE("播放完毕...");
                break;
            }
        } else {
            LOGE("播放异常退出...");
            break;
        }
    }
}

void VFFmpeg::setRenderCallback(RenderFrame renderFrame) {
    this->renderFrame = renderFrame;
}
