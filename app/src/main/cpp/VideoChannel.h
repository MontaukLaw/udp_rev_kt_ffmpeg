#ifndef MY_APPLICATION_FFMPEG_PLAYER_VIDEOCHANNEL_H
#define MY_APPLICATION_FFMPEG_PLAYER_VIDEOCHANNEL_H

#include "user_comm.h"
// 定义函数指针
typedef void (*RenderCallback)(uint8_t *, int, int, int);

class VideoChannel  : public BaseChannel{

private:
    pthread_t pid_video_decode;
    pthread_t pid_video_play;
    RenderCallback callback;

public :
    VideoChannel(AVCodecContext *codecContext);

    void start();

    void stop();

    void video_decode();

    void video_play();

    void setRenderCallback(RenderCallback callback);

    void packet_decode(AVPacket *packet);
};

#endif //MY_APPLICATION_FFMPEG_PLAYER_VIDEOCHANNEL_H
