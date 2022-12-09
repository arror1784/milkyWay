#ifndef MILKYWAY_PING_PONG_TASK_H
#define MILKYWAY_PING_PONG_TASK_H

#include "PingPongMsgQueue.h"
#include "AudioTask.h"
#include "NeoPixelTask.h"
#include "Util.h"

class PingPongTask : public Singleton<PingPongTask> {
public:
    PingPongTask();

    void sendMsg(PingPongMsgData *dataS);

    void task();

private:
    unsigned long _nextTick = 0xFFFFFFFF;
    bool _isNextSound = true;
    bool _isEnabled = false;

    PingPongMsgQueue _msgQueue;

    const long _pingPongSleepTIme = 1500;
};

#endif //MILKYWAY_PING_PONG_TASK_H
