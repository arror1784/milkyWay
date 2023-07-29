#ifndef MILKYWAY_TO_SERVER_MSG_QUEUE_H
#define MILKYWAY_TO_SERVER_MSG_QUEUE_H

#include "MsgQueue.h"

#include "Util.h"

class ToServerMsgData {
public:
    bool enable = true;
};

class ToServerMsgQueue : public MsgQueue<ToServerMsgData> {
public:
    explicit ToServerMsgQueue(int length) : MsgQueue(length, sizeof(ToServerMsgData *)) {}

    static ToServerMsgQueue &getInstance() {
        return instance_;
    }

private:
    static ToServerMsgQueue instance_;
};

#endif //MILKYWAY_TO_SERVER_MSG_QUEUE_H
