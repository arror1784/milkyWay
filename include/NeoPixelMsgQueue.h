//
// Created by jepanglee on 2022-11-26.
//

#ifndef MILKYWAY_NEOPIXELMSGQUEUE_H
#define MILKYWAY_NEOPIXELMSGQUEUE_H

#include "MsgQueue.h"
#include "NeoPixel.h"

class NeoPixelMsgData{
public:
    LightEffect list;
    NeoPixelMQEvents events;
};

class NeoPixelMsgQueue : public MsgQueue<NeoPixelMsgData> {
public:
    NeoPixelMsgQueue(int length) : MsgQueue(length, sizeof(NeoPixelMsgData*)) {
    }
};


#endif //MILKYWAY_NEOPIXELMSGQUEUE_H
