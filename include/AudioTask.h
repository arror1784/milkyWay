#ifndef MILKYWAY_AUDIO_TASK_H
#define MILKYWAY_AUDIO_TASK_H

#include "ShuffleMsgQueue.h"
#include "NeoPixelTask.h"
#include "AudioMsgQueue.h"
#include "Singleton.h"
#include "UserModeControl.h"

#define I2S_DOUT      2
#define I2S_BCLK      0
#define I2S_LRC       4

class AudioTask : public Singleton<AudioTask> {
public:
    AudioTask();

    void sendMsg(AudioMsgData *dataA);

    void task();

    void playNext();

    ShuffleMsgData *getShuffleMsg();

    void setIsDownloading(bool isDownloading);

private:
    void setNextTick(unsigned long tick);

    int _volume = 10;  // 0...21
    std::vector<int> _gains;
    TickType_t _tick = xTaskGetTickCount();
    bool _isShuffle = false;
    unsigned long _nextTick = 0xFFFFFFFF;

    AudioMsgQueue _msgQueue;
    ShuffleMsgQueue _shuffleMsgQueue;
    AudioControl _audioControl;

    const long _shuffleAudioTIme = 5500;
    const int _syncUpdateResolution = 10;
};

#endif //MILKYWAY_AUDIO_TASK_H
