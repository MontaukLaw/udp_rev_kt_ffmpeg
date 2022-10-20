cmake_minimum_required(VERSION 3.18.1)

project("marcffmpegplayer")

set(FFMPEG ${CMAKE_SOURCE_DIR}/ffmpeg)
set(RTMP ${CMAKE_SOURCE_DIR}/rtmp)

include_directories(${FFMPEG}/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${FFMPEG}/libs/${CMAKE_ANDROID_ARCH_ABI}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${RTMP}/libs/${CMAKE_ANDROID_ARCH_ABI}")

file(GLOB src_files *.cpp)

add_library( # Sets the name of the library.
        marcffmpegplayer
        SHARED
        ${src_files})

target_link_libraries( # Specifies the target library.
        marcffmpegplayer
        # 引入的库不分先后
        -Wl,--start-group
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group

        log         # 引入log库
        z           # 引入z库
        rtmp        # rtmp
        android     # 引入android库

        )