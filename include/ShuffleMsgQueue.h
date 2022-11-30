//
// Created by jepanglee on 2022-11-26.
//

#ifndef MILKYWAY_SHUFFLEMSGQUEUE_H
#define MILKYWAY_SHUFFLEMSGQUEUE_H

#include "MsgQueue.h"

#include "Util.h"

enum class ShuffleSMQEvents{
  UPDATE_ENABLE = 2
};

class ShuffleMsgData{
public:
    ShuffleSMQEvents events;
    bool enable = true;
};

class ShuffleMsgQueue : public MsgQueue<ShuffleMsgData> {
public:
    ShuffleMsgQueue(int length) : MsgQueue(length, sizeof(NeoPixelMsgData*)) {
    }
};


#endif //MILKYWAY_SHUFFLEMSGQUEUE_H
