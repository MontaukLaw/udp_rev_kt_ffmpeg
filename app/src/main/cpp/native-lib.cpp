#include "user_comm.h"

// 目的是jni在线程与函数间不能传递这些jni的环境变量, 所以需要一个全局变量来保存
JavaVM *vm = nullptr;
ANativeWindow *window = nullptr; // TODO 第三节课新增
pthread_mutex_t mutex = PTHREAD_COND_INITIALIZER; // 静态初始化 所 // TODO 第三节课新增

// 相当于一个构造函数
jint JNI_OnLoad(JavaVM *vm, void *args) {
    ::vm = vm;
    return JNI_VERSION_1_6;
}

void render_callback(uint8_t *data, int linesize, int width, int height) {
    // LOGD("rend");
    // 渲染
    pthread_mutex_lock(&mutex);
    if (!window) {
        pthread_mutex_unlock(&mutex); // 出现了问题后，必须考虑到，释放锁，怕出现死锁问题
    }

    // 设置窗口的大小，各个属性
    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);

    // 他自己有个缓冲区 buffer
    ANativeWindow_Buffer window_buffer;

    // 如果我在渲染的时候，是被锁住的，那我就无法渲染，我需要释放 ，防止出现死锁
    if (ANativeWindow_lock(window, &window_buffer, nullptr)) {
        ANativeWindow_release(window);
        window = nullptr;

        pthread_mutex_unlock(&mutex); // 解锁，怕出现死锁
        return;
    }

    // TODO 开始真正的渲染，因为window没有被锁住了，就可以rgba数据 ----> 字节对齐 渲染
    // 填充[window_buffer]  画面就出来了  ==== 【目标 window_buffer】
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_linesize = window_buffer.stride * 4;
    // linesize就是每行的字节数, 即一行数据有7680个字节

    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst_data + i * dst_linesize, data + i * linesize, dst_linesize);
        // LOGD("dst_linesize:%d  linesize:%d\n", dst_linesize, linesize);
    }
    // 数据刷新
    ANativeWindow_unlockAndPost(window); // 解锁后 并且刷新 window_buffer的数据显示画面

    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wulala_myapplicationffmpegplayer_KTPlayer_setSurfaceNative(JNIEnv *env, jobject thiz, jlong native_obj, jobject surface) {
    // 要将surface传递到VideoChannel的渲染部分
    pthread_mutex_lock(&mutex);

    // 先释放之前的显示窗口
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }

    // 创建新的窗口用于视频显示
    window = ANativeWindow_fromSurface(env, surface);

    pthread_mutex_unlock(&mutex);

}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_wulala_myapplicationffmpegplayer_KTPlayer_startRevNative(JNIEnv *env, jobject thiz) {
    auto *helper = new JNICallbackHelper(vm, env, thiz);

    auto *player = new H264Player(helper);      // 有意为之的，开辟堆空间，不能释放
    player->setRenderCallback(render_callback);

    auto *receiver = new UDP_Receiver(vm, env, thiz, player);
    receiver->init_rev();

    // 向Java层的player传递一个指针，用于后续的操作
    return reinterpret_cast<jlong>(player);
    // player->start();
}