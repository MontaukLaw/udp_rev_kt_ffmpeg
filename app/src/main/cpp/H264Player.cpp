#include "user_comm.h"

H264Player::H264Player(JNICallbackHelper *callback) {
    this->helper = callback;
    this->start_decode_play_thread();
}

// 第1个子线程, 接收数据
void *start_rev_thread(void *args) {

    // 强转回KTPlayer对象
    auto *player = static_cast<H264Player *>(args);
    player->receive_media_stream();

    return nullptr;
}


void H264Player::init_decode_context(void) {
    int ret = 0;
    AVCodec *codec = nullptr;

    LOGD("init_decode_context 1");

    codec = avcodec_find_decoder(AV_CODEC_ID_H264);

    if (codec == nullptr) {
        LOGE("JNIEncoder: Can not find codec");
        return;
    }
    LOGD("init_decode_context 2");

    codecCtx = avcodec_alloc_context3(codec);
    if (codecCtx == nullptr) {
        LOGE("JNIEncoder: Can not alloc new video_codec_ctx");
        return;
    }
    LOGD("init_decode_context 3");

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        LOGD("JNIEncoder: Failed to open decoder!");
    }
    LOGD("init_decode_context 4");

    codecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    codecCtx->width = 1920;
    codecCtx->height = 1080;
}

void H264Player::start_decode_play_thread(void) {

    init_decode_context();

    videoChannel = new VideoChannel(this->codecCtx);

    if (videoChannel) {
        // videoChannel->setRenderCallback(this->callback);
        // 设置标志位
        videoChannel->set_playing(true);
        videoChannel->start();
    }
}

void H264Player::start_decode_play_thread_back(void) {
    int ret = 0;
    isPlaying = true;

    // 获取解码器
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);

    AVCodecParameters *codecParameters = avcodec_parameters_alloc();

    codecParameters->codec_type = AVMEDIA_TYPE_VIDEO;
    codecParameters->codec_id = AV_CODEC_ID_H264;
    // codecParameters->width = 1920;
    // codecParameters->height = 1080;
    codecParameters->width = 1920;
    codecParameters->height = 1080;
    codecParameters->chroma_location = AVCHROMA_LOC_LEFT;

    // 创建上下文
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        if (helper) {
            char *errInfo = "create context failed";
            helper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL, errInfo);
        }
        return;
    }

    ret = avcodec_parameters_to_context(codecContext, codecParameters);
    if (ret < 0) {
        if (helper) {
            // 获取ffmpeg的错误提示str
            char *errInfo = av_err2str(ret);
            helper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL, errInfo);
        }
        return;
    }

    // codecContext->profile = FF_PROFILE_H264_BASELINE;
    // codecContext->gop_size = 30;
    // codecContext->time_base = (AVRational) {1, 30};
    // codecContext->framerate = (AVRational) {30, 1};
    codecContext->pix_fmt = AV_PIX_FMT_YUVJ420P; //解码器的输出格式
    // codecContext->bit_rate = 2000000;
    codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    // codecContext->frame_number = 1;
    // av_opt_set(codecContext->priv_data, "preset", "slow", 0);

    // 打开解码器
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret) {
        if (helper) {
            char *errInfo = av_err2str(ret);
            helper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL, errInfo);
        }

        return;
    }

    // 创建子线程
    // pthread_create(&pid_start, nullptr, start_rev_thread, this);

    videoChannel = new VideoChannel(codecContext);
    videoChannel->setRenderCallback(this->callback);

    if (videoChannel) {
        // 设置标志位
        videoChannel->set_playing(true);
        videoChannel->start();
    }

}

void H264Player::receive_media_stream() {

}

void H264Player::setRenderCallback(RenderCallback callback) {
    this->callback = callback;

    this->videoChannel->setRenderCallback(this->callback);
}


