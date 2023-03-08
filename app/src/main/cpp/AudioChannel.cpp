//
// Created by Yuhui Peng on 2023/3/7.
//

#include "AudioChannel.h"
#include "macro.h"


void *audioPlay(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->initOpenSLES();
    return 0;
}

void *audioDecode(void *args) {

    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);

    audioChannel->decode();
    return 0;
}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {

    auto *audioChannel = static_cast<AudioChannel *>(context);
    int datalen = audioChannel->getPcm();
    if (datalen > 0) {// 视频刚开始时判断
        (*bq)->Enqueue(bq, audioChannel->buffer, datalen);
    }
    // pcm数据 原始音频数据

}


AudioChannel::AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext,
                           AVRational time_base)
        : BaseChannel(id, javaCallHelper, codecContext, time_base) {
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_samplesize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;

    // out_sample_rate 44100 双声道2字节 outsamplesize 16位 2字节  outchannels 2字节
    buffer = (uint8_t *) malloc(out_sample_rate * out_samplesize * out_channels);
}


void AudioChannel::play() {
    swr_context = swr_alloc_set_opts(
            0, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, out_sample_rate,
            avCodecContext->channel_layout,
            avCodecContext->sample_fmt,
            avCodecContext->sample_rate, 0, 0
    );

    swr_init(swr_context);

    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
// 创建初始化线程
    pthread_create(&pid_audio_play, NULL, audioPlay, this);

// 创建初始化音频解码线程
    pthread_create(&pid_audio_decode, NULL, audioDecode, this);

}

void AudioChannel::stop() {

}

void AudioChannel::initOpenSLES() {
    // 音频引擎
    SLEngineItf engineInterface = NULL;
    // 音频对象
    SLObjectItf engineObject = NULL;
    // 混音器
    SLObjectItf outputMixObject = NULL;

    // 播放器
    SLObjectItf bqPlayerObject = NULL;
    // 回调接口
    SLPlayItf bqPlayerInterface = NULL;
    // 缓冲队列
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = NULL;

    // --------------初始化播放音频
    SLresult lresult = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != lresult) {
        return;
    }

    lresult = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != lresult) {
        return;
    }

    // 音频接口
    lresult = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != lresult) {
        return;
    }

    // ---------------初始化混音器
    lresult = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, 0, 0);
    if (SL_RESULT_SUCCESS != lresult) {
        return;
    }
    // 初始化混音器OutputMixObject
    lresult = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != lresult) {
        return;
    }

    // ---------------初始化播放器
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,// 播放数据格式
            2,
            SL_SAMPLINGRATE_44_1, // hz
            SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, // 位数两个一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 声道范围
            SL_BYTEORDER_LITTLEENDIAN //
    };
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    SLDataLocator_OutputMix loc_outmix = {
            SL_DATALOCATOR_OUTPUTMIX, // 看变量注释，就写这个就行
            outputMixObject
    };
    SLDataSink audioSnk = {&loc_outmix, NULL};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};

    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    (*engineInterface)->CreateAudioPlayer(
            engineInterface,
            &bqPlayerObject,    // 播放器
            &audioSrc,  // 播放器参数 播放缓冲队列 播放格式
            &audioSnk,  //
            1,  // 播放接口回调个数
            ids,    // 设置播放队列id
            req // 是否采取内置播放器
    );

    // 初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    // 获取播放接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);

    // -----------------初始化缓冲队列
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);

    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);

    bqPlayerCallback(bqPlayerBufferQueue, this);
    LOGE("手动调用 packet:%d", this->pkt_queue.size());

}

void AudioChannel::decode() {

    AVPacket *packet = 0;
    while (isPlaying) {
        // 音频packet
        int ret = pkt_queue.get(packet);

        if (!isPlaying) {
            break;
        }

        if (!ret) {
            continue;
        }

        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAVPacket(packet);

        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }

        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }

        while (frame_queue.size() > 100 && isPlaying) {
            av_usleep(1000 * 10);
            continue;
        }
        frame_queue.push(frame);

    }

}

int AudioChannel::getPcm() {
    AVFrame *frame = 0;
    int data_size = 0;
    while (isPlaying) {
        int ret = frame_queue.get(frame);

        if (!isPlaying) {
            break;
        }

        if (!ret) {
            continue;
        }

        uint64_t dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swr_context, frame->sample_rate) + frame->nb_samples,
                out_sample_rate,
                frame->sample_rate,
                AV_ROUND_UP
        );

        // 转换数据，返回值为转换后的sample个数
        int nb = swr_convert(swr_context, &buffer, dst_nb_samples,
                             (const uint8_t **) frame->data, frame->nb_samples);

        // 转换后多少数据 buffer size 44100*2*2
        data_size = nb * out_channels * out_samplesize;

        // 计算clock
        clock = frame->pts * av_q2d(time_base);

        break;

    }

    releaseAVFrame(frame);

    return data_size;
}
