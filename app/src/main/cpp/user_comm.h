#ifndef MY_APPLICATION_FFMPEG_PLAYER_USER_COMM_H
#define MY_APPLICATION_FFMPEG_PLAYER_USER_COMM_H

#include <jni.h>
#include <string>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <android/native_window_jni.h> // ANativeWindow 用来渲染画面的 == Surface对象

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
}

#include "safe_queue.h"
#include "BaseChannel.h"
#include "VideoChannel.h"
#include "util.h"
#include "JNICallbackHelper.h"
#include "H264Player.h"
#include "UDP_Receiver.h"


#endif //MY_APPLICATION_FFMPEG_PLAYER_USER_COMM_H
