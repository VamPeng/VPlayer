//
// Created by Yuhui Peng on 2023/3/7.
//

#ifndef VPLAYER_AUDIOCHANNEL_H
#define VPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"

#include <SLES/OpenSLES_Android.h>

extern "C" {

#include "libavutil/time.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
}

class AudioChannel : public BaseChannel {

public:
    AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext,
                 AVRational time_base);

    void play() override;

    void stop() override;

    void decode();

    void initOpenSLES();

    int getPcm();

private:
    pthread_t pid_audio_play;
    pthread_t pid_audio_decode;

    SwrContext *swr_context = NULL;
    int out_channels;
    int out_samplesize;
    int out_sample_rate;

public:
    uint8_t *buffer;

};

#endif //VPLAYER_AUDIOCHANNEL_H
