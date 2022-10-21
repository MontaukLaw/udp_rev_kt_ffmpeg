cmake_minimum_required(VERSION 3.18.1)

project("marcffmpegplayer")

# 设置变量方便下面使用
set(FFMPEG ${CMAKE_SOURCE_DIR}/ffmpeg)
set(RTMP ${CMAKE_SOURCE_DIR}/rtmp)

include_directories(${FFMPEG}/include)

# 指明动态库的存放位置
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${FFMPEG}/libs/${CMAKE_ANDROID_ARCH_ABI}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${RTMP}/libs/${CMAKE_ANDROID_ARCH_ABI}")

# 指明源文件的位置
file(GLOB src_files *.cpp)

# 将你的cpp编译成一个动态库供java使用
add_library(
        marcffmpegplayer   # 这个是动态库的名字, 你要在java中使用这个动态库, 就需要这个名字
        SHARED
        ${src_files})

# 指明需要链接的库, 就是依赖
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