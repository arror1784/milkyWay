#ifndef MILKYWAY_AUDIO_TASK_H
#define MILKYWAY_AUDIO_TASK_H

#include "PingPongMsgQueue.h"
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

    void handlePlayStatus(bool status = true);

    void setIsSDAccessing(bool isSDAccessing);

    const Sound &getCurrentSound();

    void setShouldChangeSound(bool shouldChangeSound);

private:
    bool _isEnabled = false;
    int _volume = 10;  // 0...21
    std::vector<int> _gains;
    TickType_t _tick = xTaskGetTickCount();
    bool _shouldChangeSound = true;

    AudioMsgQueue _msgQueue;
    AudioControl _audioControl;

    const int _syncUpdateResolution = 10;
};

#endif //MILKYWAY_AUDIO_TASK_H
