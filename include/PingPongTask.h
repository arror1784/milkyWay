#ifndef MILKYWAY_PING_PONG_TASK_H
#define MILKYWAY_PING_PONG_TASK_H

#include "PingPongMsgQueue.h"
#include "AudioTask.h"
#include "NeoPixelTask.h"
#include "Util.h"

enum class EPingPongStatus {
    SLEEP, AUDIO, NEO_PIXEL
};

class PingPongTask : public Singleton<PingPongTask> {
public:
    void task();

private:
    unsigned long _nextTick = 0xFFFFFFFF;
    EPingPongStatus _status = EPingPongStatus::SLEEP;
    EPingPongStatus _nextStatus = EPingPongStatus::SLEEP;
    bool _isEnabled = false;
    int _neoPixelCount = 0;

    const int _neoPixelCountCycle = 4;
    const long _pingPongSleepTIme = 1500;
    const long _pingPongAudioTIme = 5500;
};

#endif //MILKYWAY_PING_PONG_TASK_H
