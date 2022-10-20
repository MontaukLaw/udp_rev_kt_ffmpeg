#include "user_comm.h"

JNICallbackHelper::JNICallbackHelper(JavaVM *vm, JNIEnv *env, jobject instance) {
    this->vm = vm;
    this->env = env;
    this->instance = env->NewGlobalRef(instance);
    jclass clazz = env->GetObjectClass(instance);
    if (!clazz) {
        return;
    }
    // jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V");
    // java层处理错误的方法名为onError，参数为int类型，返回值为void
    // jmd_error = env->GetMethodID(clazz, "onError", "(ILjava/lang/String;)V");
}

JNICallbackHelper::~JNICallbackHelper() {
    env->DeleteGlobalRef(instance);
    vm = nullptr;
    instance = nullptr;
    env = nullptr;
}

void JNICallbackHelper::onPrepared(int thread_mode) {
}

void JNICallbackHelper::onError(int thread_mode, int error_code, char *ffmpegErrorMsg) {

}

