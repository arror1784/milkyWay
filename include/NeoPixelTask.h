#ifndef MILKYWAY_NEOPIXEL_TASK_H
#define MILKYWAY_NEOPIXEL_TASK_H

#include "NeoPixelMsgQueue.h"
#include "NeoPixel.h"
#include "UserModeControl.h"

#include "Util.h"

#define LED_PIN 32
#define LED_LENGTH 24

class NeoPixelTask : public Singleton<NeoPixelTask> {
public:
    NeoPixelTask();

    void sendMsg(NeoPixelMsgData *dataN);

    const LightEffect &getLightEffect(ELightMode mode);

    unsigned int getSpeed();

    void setLightEffect(const LightEffect &lightEffect);

    void addNextTick(unsigned long speed);

    void task();

    void ticked();

    void finishCycle();

private:
    void setColorSet();

    void setNextTick(unsigned long tick);

    void refreshMode();

    ELightMode _mode = ELightMode::None;

    unsigned long _previousSpeed = 0;
    unsigned long _nextTick = 0xFFFFFFFF;

    bool _enabled = false;
    NeoPixel _neoPixel;
    bool _isShuffle = false;

    int _count = -1;

    NeoPixelMsgQueue _msgQueue;

    LightEffect _breathingLightEffect;
    LightEffect _blinkingLightEffect;
    LightEffect _colorChangeLightEffect;

    LightEffect _defaultBreathingLightEffect;
    LightEffect _defaultBlinkingLightEffect;
    LightEffect _defaultColorChangeLightEffect;

    const std::vector<unsigned int> _breathingSpeeds{
        1500, 2500, 4000, 5000, 6000, 8000, 10000, 12000, 15000
    };

    const std::vector<unsigned int> _blinkingSpeeds{
        500, 1000, 1500, 2000, 2500, 3000, 5000
    };

    const int _oneCycleCount = 4;
};

#endif //MILKYWAY_NEOPIXEL_TASK_H
