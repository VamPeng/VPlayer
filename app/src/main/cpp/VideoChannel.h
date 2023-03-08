//
// Created by Yuhui Peng on 2023/3/7.
//

#ifndef VPLAYER_VIDEOCHANNEL_H
#define VPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"
#include "AudioChannel.h"

typedef void (*RenderFrame)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel {

public:
    VideoChannel(int id,
                 JavaCallHelper *javaCallHelper, AVCodecContext *codecContext,AVRational avRational_);

    virtual void play();

    virtual void stop();

    void decodePacket();

    void synchronizeFrame();

    void setFps(double fps_);

    RenderFrame renderFrame;


    void setRenderFrame(RenderFrame renderFrame1);
private:
    pthread_t pid_video_play;

    pthread_t pid_synchronize;

    double fps;

public:
    AudioChannel *audioChannel;

};


#endif //VPLAYER_VIDEOCHANNEL_H
