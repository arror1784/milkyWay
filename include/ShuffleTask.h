#ifndef MILKYWAY_SHUFFLE_TASK_H
#define MILKYWAY_SHUFFLE_TASK_H

#include "ShuffleMsgQueue.h"
#include "AudioTask.h"
#include "NeoPixelTask.h"
#include "Util.h"

class ShuffleTask : public Singleton<ShuffleTask> {
public:
    ShuffleTask();

    void sendMsg(ShuffleMsgData *dataS);

    void task();

private:
    unsigned long _nextTick = 0xFFFFFFFF;
    bool _isNextSound = true;
    bool _isEnabled = false;

    ShuffleMsgQueue _msgQueue;

    const long _shuffleSleepTIme = 1500;
};

#endif //MILKYWAY_SHUFFLE_TASK_H
