//
// Created by jepanglee on 2022-11-26.
//

#ifndef MILKYWAY_NEOPIXELMSGQUEUE_H
#define MILKYWAY_NEOPIXELMSGQUEUE_H

#include "MsgQueue.h"
#include "NeoPixel.h"

#include "Util.h"

enum class NeoPixelMQEvents{
  UPDATE_MODE = 0,
  UPDATE_EFFECT = 1,
  UPDATE_ENABLE = 2,
};

class NeoPixelMsgData{
public:
    LightEffect list;
    NeoPixelMQEvents events;
    ELightMode mode;
    bool enable = true;
};

class NeoPixelMsgQueue : public MsgQueue<NeoPixelMsgData> {
public:
    NeoPixelMsgQueue(int length) : MsgQueue(length, sizeof(NeoPixelMsgData*)) {
    }
};


#endif //MILKYWAY_NEOPIXELMSGQUEUE_H
