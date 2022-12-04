#ifndef MILKYWAY_SHUFFLEMSGQUEUE_H
#define MILKYWAY_SHUFFLEMSGQUEUE_H

#include "MsgQueue.h"

#include "Util.h"

enum class EShuffleSMQEvent {
    FINISH_SOUND,
    FINISH_NEO_PIXEL,
    UPDATE_ENABLE
};

class ShuffleMsgData {
public:
    EShuffleSMQEvent events;
    bool enable = true;
};

class ShuffleMsgQueue : public MsgQueue<ShuffleMsgData> {
public:
    ShuffleMsgQueue(int length) : MsgQueue(length, sizeof(ShuffleMsgData *)) {
    }
};

#endif //MILKYWAY_SHUFFLEMSGQUEUE_H
