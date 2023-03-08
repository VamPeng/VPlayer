cmake_minimum_required(VERSION 3.22.1)

project("vplayer")

add_library(
        vplayer
        SHARED
        native-lib.cpp VFFmpeg.cpp JavaCallHelper.cpp AudioChannel.cpp VideoChannel.cpp)

set(my_lib_dir ${CMAKE_SOURCE_DIR}/../../../libs)
set(my_head_dir ${CMAKE_SOURCE_DIR}/ffincludes)

include_directories(${my_head_dir})

find_library(
        log-lib
        log)

add_library(avcodec
        SHARED
        IMPORTED)
set_target_properties(
        avcodec
        PROPERTIES IMPORTED_LOCATION
        ${my_lib_dir}/${CMAKE_ANDROID_ARCH_ABI}/libavcodec.so
)

add_library(avfilter
        SHARED
        IMPORTED)
set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${my_lib_dir}/${CMAKE_ANDROID_ARCH_ABI}/libavfilter.so
)

add_library(avformat
        SHARED
        IMPORTED)
set_target_properties(
        avformat
        PROPERTIES IMPORTED_LOCATION
        ${my_lib_dir}/${CMAKE_ANDROID_ARCH_ABI}/libavformat.so
)

add_library(avutil
        SHARED
        IMPORTED)
set_target_properties(
        avutil
        PROPERTIES IMPORTED_LOCATION
        ${my_lib_dir}/${CMAKE_ANDROID_ARCH_ABI}/libavutil.so
)

add_library(swresample
        SHARED
        IMPORTED)
set_target_properties(
        swresample
        PROPERTIES IMPORTED_LOCATION
        ${my_lib_dir}/${CMAKE_ANDROID_ARCH_ABI}/libswresample.so
)

add_library(swscale
        SHARED
        IMPORTED)
set_target_properties(
        swscale
        PROPERTIES IMPORTED_LOCATION
        ${my_lib_dir}/${CMAKE_ANDROID_ARCH_ABI}/libswscale.so
)

target_link_libraries(
        vplayer
        avcodec avfilter avformat avutil swresample swscale
        z
        OpenSLES
        android
        log
)