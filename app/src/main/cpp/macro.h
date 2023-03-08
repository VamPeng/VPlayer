//
// Created by Yuhui Peng on 2023/3/7.
//

#ifndef VPLAYER_MACRO_H
#define VPLAYER_MACRO_H

#define THREAD_Main 1
#define THREAD_CHILD 2

#define FFMPEG_CAN_NOT_OPEN_URL 1
#define FFMPEG_CAN_NOT_FIND_STREAMS 2
#define FFMPEG_FIND_DECODER_FAIL 3
#define FFMPEG_ALLOC_CODEC_CONTEXT_FAIL 4
#define FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL 6
#define FFMPEG_OPEN_DECODER_FAIL 7
#define FFMPEG_NO_MEDIA 8

#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "VAM", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "VAM", __VA_ARGS__)

#endif //VPLAYER_MACRO_H
