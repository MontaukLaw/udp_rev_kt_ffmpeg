#ifndef MY_APPLICATION_FFMPEG_PLAYER_H264PLAYER_H
#define MY_APPLICATION_FFMPEG_PLAYER_H264PLAYER_H

#include "user_comm.h"

class H264Player {

private :
    bool isPlaying = false;

    pthread_t pid_start;
    RenderCallback callback;
    JNICallbackHelper *helper = nullptr;
    AVCodecContext *codecCtx = nullptr;

public:
    VideoChannel *videoChannel = nullptr;

public:

    H264Player(JNICallbackHelper *callback);

    void start_decode_play_thread(void);

    void setRenderCallback(RenderCallback callback);

    void receive_media_stream();

    void init_decode_context();

    void start_decode_play_thread_back();

};

#endif //MY_APPLICATION_FFMPEG_PLAYER_H264PLAYER_H
