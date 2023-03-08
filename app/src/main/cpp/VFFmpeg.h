//
// Created by Yuhui Peng on 2023/3/7.
//

#ifndef VPLAYER_VFFMPEG_H
#define VPLAYER_VFFMPEG_H

#include "pthread.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/time.h"
}

#include "JavaCallHelper.h"

#include "VideoChannel.h"
#include "AudioChannel.h"

class VFFmpeg {

public:
    VFFmpeg(JavaCallHelper *javaCallHelper_, const char *dataSource);

    ~VFFmpeg();

    void prepare();

    void prepareFFmpeg();

    void start();

    void play();

    void setRenderCallback(RenderFrame renderFrame);

private:
    bool isPlaying;
    pthread_t pid_prepare;
    pthread_t pid_play;
    char *url;
    AVFormatContext *formatContext;
    JavaCallHelper *javaCallHelper;

    void childThreadError(int code);

    RenderFrame renderFrame;
    VideoChannel *videoChannel;
    AudioChannel *audioChannel;
};


#endif //VPLAYER_VFFMPEG_H
