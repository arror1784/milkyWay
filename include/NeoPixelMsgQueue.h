#ifndef MILKYWAY_NEOPIXELMSGQUEUE_H
#define MILKYWAY_NEOPIXELMSGQUEUE_H

#include "MsgQueue.h"
#include "NeoPixel.h"
#include "UserModeControl.h"

#include "Util.h"

#define LED_PIN 32
#define LED_LENGTH 24

enum class ENeoPixelMQEvent {
    UPDATE_MODE = 0,
    UPDATE_EFFECT = 1,
    UPDATE_ENABLE = 2,
    UPDATE_SYNC = 3
};

class NeoPixelMsgData {
public:
    LightEffect lightEffect;
    ENeoPixelMQEvent events;
    ELightMode mode;
    bool enable = true;
    uint8_t sync = 0;
    bool isShuffle = false;
};

class NeoPixelMsgQueue : public MsgQueue<NeoPixelMsgData> {
public:
    NeoPixelMsgQueue(int length) : MsgQueue(length, sizeof(NeoPixelMsgData *)) {}
};
#endif //MILKYWAY_NEOPIXELMSGQUEUE_H
