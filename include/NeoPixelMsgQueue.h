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
  UPDATE_SYNC = 3
};

class NeoPixelMsgData{
public:
    LightEffect lightEffect;
    NeoPixelMQEvents events;
    ELightMode mode;
    bool enable = true;
    uint8_t sync = 0;
};

class NeoPixelMsgQueue : public MsgQueue<NeoPixelMsgData> {
public:
    NeoPixelMsgQueue(int length) : MsgQueue(length, sizeof(NeoPixelMsgData*)) {
    }
};


#endif //MILKYWAY_NEOPIXELMSGQUEUE_H
