#ifndef MILKYWAY_NEOPIXEL_TASK_H
#define MILKYWAY_NEOPIXEL_TASK_H

#include "ShuffleMsgQueue.h"
#include "NeoPixelMsgQueue.h"
#include "NeoPixel.h"
#include "UserModeControl.h"

#include "Util.h"

#define LED_PIN 32
#define LED_LENGTH 50

class NeoPixelTask : public Singleton<NeoPixelTask> {
public:
    NeoPixelTask();

    ShuffleMsgData *getShuffleMsg();

    void sendMsg(NeoPixelMsgData *dataN);

    void updateCustomLightEffect(const LightEffect &lightEffect);

    void setCurrentLightEffect(ELightMode mode);

    void task();

private:
    void ticked();

    // breath 기능 중 싸이클이 끝날때마다 true를 반환합니다.
    bool breath();

    // blink 기능 중 싸이클이 끝날때마다 true를 반환합니다.
    bool blink();

    void sync();

    void finishCycle();

    void reset();

    void refreshColorSet();

    void refreshSpeed();

    void refreshNextTick();

    ELightMode _mode = ELightMode::None;

    unsigned long _speed = 0;
    unsigned long _nextTick = 0xFFFFFFFF;

    uint8_t _sync = 0;
    bool _isSyncMode = false;
    NeoPixel _neoPixel;
    bool _isShuffle = false;

    // -1일 경우 무한 반복
    int _count = -1;

    NeoPixelMsgQueue _msgQueue;
    ShuffleMsgQueue _shuffleMsgQueue;

    LightEffect *_currentLightEffect;

    LightEffect _breathingLightEffect;
    LightEffect _blinkingLightEffect;
    LightEffect _colorChangeLightEffect;

    LightEffect _defaultBreathingLightEffect;
    LightEffect _defaultBlinkingLightEffect;
    LightEffect _defaultColorChangeLightEffect;

    std::vector<int> _colorIndexes;
    std::vector<int> _lightEffectIndexes;
    const int _lightEffectModeCount = 3;

    const std::vector<unsigned int> _breathingSpeeds{
        1500, 2500, 4000, 5000, 6000, 8000, 10000, 12000, 15000
    };

    const std::vector<unsigned int> _blinkingSpeeds{
        500, 1000, 1500, 2000, 2500, 3000, 5000
    };

    const int _oneCycleCount = 4;
};

#endif //MILKYWAY_NEOPIXEL_TASK_H
