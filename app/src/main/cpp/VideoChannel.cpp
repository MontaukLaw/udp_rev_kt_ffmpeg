#include "user_comm.h"
#include "VideoChannel.h"


VideoChannel::VideoChannel(AVCodecContext *codecContext)
        : BaseChannel(codecContext) {

}

// 解码后就进行播放
void VideoChannel::video_play() {
    AVFrame *frame = nullptr;
    uint8_t *dst_data[4];   // 原始yuv数据, 非常大的一个数组
    int dst_linesize[4];

    // 解码器上下文是BaseChannel的一个成员变量
    av_image_alloc(dst_data, dst_linesize, codecContext->width, codecContext->height,
                   AV_PIX_FMT_RGBA, 1);

    SwsContext *swsContext = sws_getContext(codecContext->width, codecContext->height,
                                            codecContext->pix_fmt,  // 输入的格式
                                            codecContext->width, codecContext->height,
                                            AV_PIX_FMT_RGBA,       // 输出的格式 RGBA, 安卓只能支持RGBA
                                            SWS_BILINEAR, nullptr, nullptr, nullptr);

    while (isPlaying) {
        LOGD("video_play\n");
        int ret = frames.pop_from_queue(frame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        LOGD("video_play 2\n");
        // 格式转换, 将yuv420p转换成rgba
        sws_scale(
                // 输入
                swsContext, frame->data, frame->linesize, 0, codecContext->height,
                // 输出
                dst_data, dst_linesize);

        // 在渲染之前, 做音视频同步
        // 视频要根据音频的时间戳决定是否需要延迟渲染.

        // native window
        // 开始渲染
        LOGD("video_play 3\n");
        if (callback == nullptr) {
            LOGE("wtf");
        }

        callback(dst_data[0], dst_linesize[0], codecContext->width, codecContext->height);

        LOGD("video_play 4\n");
        // 渲染完之后, 释放frame内存
        av_frame_unref(frame);
        release_av_frame(&frame);
    }

    // 全部释放, 道德所致
    av_frame_unref(frame);
    release_av_frame(&frame);
    isPlaying = false;
    av_freep(&dst_data[0]);
    sws_freeContext(swsContext);

}

// read file的消费者, 同时是play的生产者, 双重身份.
// 本质上, 这里应该是同步的, 不存在异步过程
void VideoChannel::video_decode() {
    AVPacket *packet = nullptr;

    while (isPlaying) {

        // 当数据包读取的速度过快, 而来不及进行解码的时候, 这里需要放慢解码的速度.
        // 注意这里要观察的是frames队列, 而不是packets队列
        if (isPlaying && frames.if_queue_full()) {
            av_usleep(10 * 1000);  // 10ms
            continue;
        }

        LOGD("video_decode\n");
        int ret = packets.pop_from_queue(packet);  // 阻塞式获取队列中的数据包
        // 当外界要停止这个线程的时候，会把isPlaying设置为false，这个时候就会跳出循环，释放packet
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            LOGE("video_decode pop_from_queue failed\n");
            continue;
        }
        LOGD("ready to decode packet:%d\n", packet->size);

        // 发送到ffmpeg的缓冲区
        ret = avcodec_send_packet(codecContext, packet);
        if (ret < 0) {
            LOGE("avcodec_send_packet failed ret:%d\n", ret);

            av_packet_unref(packet);
            release_av_packet(&packet);
            continue;
        }

        //avcodec_decode_video2(codecContext, frame, &got_frame, packet);

        LOGD("packet sent");
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            // 如果B帧参考后面的数据失败, 可能是P帧还没有出来, 就会出现这个错误
            LOGE("B frame error");
            continue;
        } else if (ret != 0) {
            if (frame) {
                LOGE("frame get error");
                av_frame_free(&frame);
            }
            break;
        }

        // 保证播放队列不会堆满
//        if (frames.if_queue_full()) {
//
//            LOGD("video packet queue is full, drop old frame");
//            AVFrame *frameToDrop = nullptr;
//            frames.pop_from_queue(frameToDrop);
//            av_frame_unref(frameToDrop);
//            av_frame_free(&frameToDrop);
//        }
        // LOGD("video_decode success\n");
        // 将这一帧放入frames队列中
        frames.insert_to_queue(frame);

        // av_freep(packet->data);
        // av_packet_free_side_data(packet);
        // av_packet_unref(packet);
        // 把自己的packet释放掉, 避免内存泄漏
        release_av_packet(&packet);
        // av_packet_unref(packet);
        // release_av_packet(&packet);
    }

    av_packet_unref(packet);
    release_av_packet(&packet);
}


void *task_video_decode(void *args) {
    auto *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_decode();
    return nullptr;
}

void *task_video_play(void *args) {
    auto *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_play();
    return nullptr;
}

void VideoChannel::start() {

    isPlaying = true;

    packets.set_work(true);
    // 第一个线程， 取出队列的数据包，解码成数据帧
    pthread_create(&pid_video_decode, nullptr, task_video_decode, this);

    // frames.set_work(true);
    // 第二个线程, 取出解码后的数据进行播放
    pthread_create(&pid_video_play, nullptr, task_video_play, this);

}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->callback = callback;
}
