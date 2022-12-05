#ifndef MILKYWAY_AUDIO_TASK_H
#define MILKYWAY_AUDIO_TASK_H

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

    void setIsDownloading(bool isDownloading);

private:
    int volume = 10;  // 0...21
    std::vector<int> gains;
    TickType_t tick = xTaskGetTickCount();
    bool isShuffle = false;
    unsigned long nextTick = 0xFFFFFFFF;

    AudioMsgQueue _msgQueue;
    AudioControl _audioControl;

    const long _shuffleAudioTIme = 5500;
    const int _syncUpdateResolution = 10;
};

#endif //MILKYWAY_AUDIO_TASK_H
