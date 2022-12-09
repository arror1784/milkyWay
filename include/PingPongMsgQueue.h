#ifndef MILKYWAY_PING_PONG_MSG_QUEUE_H
#define MILKYWAY_PING_PONG_MSG_QUEUE_H

#include "MsgQueue.h"

#include "Util.h"

enum class EPingPongMQEvent {
    FINISH_SOUND,
    FINISH_NEO_PIXEL,
    UPDATE_ENABLE
};

class PingPongMsgData {
public:
    EPingPongMQEvent events;
    bool enable = true;
};

class PingPongMsgQueue : public MsgQueue<PingPongMsgData> {
public:
    PingPongMsgQueue(int length) : MsgQueue(length, sizeof(PingPongMsgData *)) {
    }
};

#endif //MILKYWAY_PING_PONG_MSG_QUEUE_H
