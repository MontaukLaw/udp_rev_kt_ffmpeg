#include "user_comm.h"
#include "UDP_Receiver.h"


extern "C" {
#include <sys/socket.h>
}

frame_type_e UDP_Receiver::get_frame_type(char *data, int packetLen) {

    if (if_frame_started(data)) {
        if ((data[4] == 0x67)) {
            return SPS_FRAME;
        } else if (packetLen == 8 && (data[4] == 0x68)) {
            return PPS_FRAME;
        } else if (packetLen == 9 && (data[4] == 0x06)) {
            return E_FRAME;
        } else if (data[4] == 0x65) {
            return I_FRAME;
        } else {
            return D_START_PACKET;
        }
    } else {
        return D_REST_PACKET;
    }
}

bool UDP_Receiver::if_frame_started(char *data) {

    if (data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 1) {
        return true;
    }
    return false;
}


void *rev_thread(void *args) {
    char packetBuf[UDP_PACKET_LEN];   // udp包最大是1400
    int recvLen;
    int dataFrameLen = 0;
    // 强转为receiver对象指针, 为了取到sockfd
    auto *receiver = static_cast<UDP_Receiver *>(args);

    char dataFrameBuf[DATA_BUFFER_LEN];

    while (true) {
        // 读取udp数据包
        recvLen = read(receiver->sockfd, (char *) packetBuf, UDP_PACKET_LEN);

        if (recvLen < 0) {
            LOGD("recvfrom failed");
            break;
        }

        if (recvLen > 0) {
            // LOGD("Got data: %d bytes", recvLen);

            // 获取数据包的类型
            // 这里的注释暂时不要删除, 知道显示确定无问题
            frame_type_e frameType = receiver->get_frame_type(packetBuf, recvLen);

            // 经过分析, 只有I帧/P帧即数据帧才发送到解码器
            if (SPS_FRAME == frameType) {
                if (!receiver->ifStartRender) {
                    LOGD("Start render");
                    receiver->ifStartRender = true;
                }
                LOGD("SFrame insert buffered data into packet queue");
                // 将已经缓存的数据帧插入到队列中
                receiver->insert_data_into_players_packet_queue(dataFrameBuf, dataFrameLen);

                dataFrameLen = 0;
                // 加入缓存, 跟I帧一起发送
                memcpy(&dataFrameBuf[dataFrameLen], packetBuf, recvLen);
                dataFrameLen = recvLen;

            } else if (PPS_FRAME == frameType) {
                // 加入缓存, 跟I帧一起发送
                memcpy(&dataFrameBuf[dataFrameLen], packetBuf, recvLen);
                dataFrameLen = dataFrameLen + recvLen;
            } else if (I_FRAME == frameType) {

                // 加入缓存
                memcpy(&dataFrameBuf[dataFrameLen], packetBuf, recvLen);
                dataFrameLen = dataFrameLen + recvLen;

            } else if (D_START_PACKET == frameType) {

                // D帧开始了, 就把之前的先发送
                receiver->insert_data_into_players_packet_queue(dataFrameBuf, dataFrameLen);

                dataFrameLen = 0;
                // 加入缓存
                memcpy(&dataFrameBuf[dataFrameLen], packetBuf, recvLen);
                dataFrameLen = recvLen;

            } else if (D_REST_PACKET == frameType) {
                // 数据帧的中间部分, 直接缓存
                memcpy(&dataFrameBuf[dataFrameLen], packetBuf, recvLen);
                dataFrameLen = dataFrameLen + recvLen;
            }

            /*

            if (frameType == SPS_FRAME) {
                if (!receiver->ifStartRender) {
                    LOGD("Start render");
                    receiver->ifStartRender = true;
                }
                LOGD("SFrame insert buffered data into packet queue");
                // 将已经缓存的数据帧插入到队列中
                receiver->insert_data_into_players_packet_queue(dataFrameBuf, dataFrameLen);

                // 并将新的数据也放入队列中
                LOGD("SFrame insert this packet into packet queue");
                receiver->insert_data_into_players_packet_queue(packetBuf, recvLen);

            } else if (frameType == PPS_FRAME) {
                LOGD("insert this data into packet queue");
                receiver->insert_data_into_players_packet_queue(packetBuf, recvLen);

            } else if (D_START_PACKET == frameType) {
                LOGD("D_START_PACKET insert last data to queue");
                receiver->insert_data_into_players_packet_queue(dataFrameBuf, dataFrameLen);

                // LOGD("buffer this data");
                memcpy(&dataFrameBuf[dataFrameLen], packetBuf, recvLen);
                dataFrameLen = recvLen;

            } else if (D_REST_PACKET == frameType) {
                //LOGD("buffer this data");
                memcpy(&dataFrameBuf[dataFrameLen], packetBuf, recvLen);
                dataFrameLen = dataFrameLen + recvLen;

            } else if (I_FRAME == frameType) {

                LOGD("I_Frame buffer this data");
                memcpy(&dataFrameBuf[dataFrameLen], packetBuf, recvLen);
                dataFrameLen = recvLen;
            }
            */
        }
        usleep(1);
    }
    return nullptr;
}

void UDP_Receiver::init_rev(void) {
    LOGD("start rev ");

    int ret = 0;
    struct sockaddr_in servaddr;

    if ((this->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        LOGE("socket creation failed");
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(this->sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));

    if (ret < 0) {
        LOGE("bind failed");
        return;
    }

    // socklen_t clintAddrSize = sizeof(sockaddr_in);

    pthread_create(&revPid, nullptr, rev_thread, this);   // 将jniContext当参数传进去

}

UDP_Receiver::UDP_Receiver(JavaVM *jvm, JNIEnv *env, jobject const &instance, H264Player *player) {
    this->jvm = jvm;
    this->env = env;
    this->instance = env->NewGlobalRef(instance);
    jclass clazz = env->GetObjectClass(instance);
    if (!clazz) {
        return;
    }

    this->player = player;

}

void UDP_Receiver::insert_data_into_players_packet_queue(char *data, int dataLen) {
    if (this->ifStartRender) {
        AVPacket *packet = av_packet_alloc();
        //packet->data = (uint8_t *) av_malloc(dataLen);
        packet->data = (uint8_t *) data;
        // memcpy(packet->data, data, dataLen);
        packet->size = dataLen;
        packet->stream_index = 0;


        this->player->videoChannel->packet_decode(packet);

        av_packet_unref(packet);
        av_packet_free(&packet);
        packet = nullptr;
    }
}

void UDP_Receiver::insert_data_into_players_packet_queue_old(char *data, int dataLen) {
    if (this->ifStartRender) {

        // LOGD("insert data into video channel packet queue");

        AVPacket *packet = av_packet_alloc();
        packet->data = (uint8_t *) av_malloc(dataLen);
        memcpy(packet->data, data, dataLen);
        packet->size = dataLen;
        packet->stream_index = 0;

        if (this->player->videoChannel->packets.if_queue_full()) {

            LOGD("video packet queue is full, drop old packet");
            AVPacket *packetToDrop = nullptr;
            this->player->videoChannel->packets.pop_from_queue(packetToDrop);
            av_packet_unref(packetToDrop);
            av_packet_free(&packetToDrop);
        }

        if (this->player->videoChannel->packets.get_queue_size()) {
            LOGD("queue size now is %d", this->player->videoChannel->packets.get_queue_size());
        }

        // this->player->videoChannel->packets.insert_to_queue(packet);

        this->player->videoChannel->packet_decode(packet);

        av_packet_unref(packet);
        av_packet_free(&packet);
        packet = nullptr;
    }
}

