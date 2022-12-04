#ifndef ESP32_LIB_NEOPIXEL_H
#define ESP32_LIB_NEOPIXEL_H

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>

#include "Util.h"
#include "SDUtil.h"

class ColorSet {
public:
    long id;
    std::vector<uint32_t> colors;
};

class LightEffect {
public:
    long id = 0;
    std::vector<ColorSet> colorSets;
    ELightMode mode = ELightMode::Blinking;
    bool isRandomColor;
    long speed;
    bool isRandomSpeed;
};

class Neopixel {
public:
    Neopixel(int nLed, int pin, neoPixelType type);

    void begin();

    void plotColorSet(const LightEffect &lightEffect, int colorSetIndex, uint8_t br);

    void plotColorSet(const LightEffect &lightEffect, int colorSetIndex);

    void colorChange(int changes);

    void dim(int dims);

    void blink(int blinks);

    void sync(uint8_t per);

    void setLightEffect(const LightEffect &lightEffect);

    void changeMode(ELightMode mode);

    void loop();

    void setEnable(bool enable) { _enable = enable; };

    const LightEffect &getLightEffect(ELightMode mode);

private:
    Adafruit_NeoPixel _strip;
    int _pin;
    int _nLed;

    int _mixCount = 0;
    int _mixMode = 0;
    const int _MaxMixCount = 4;
    int _colorPresetIndex = 0;

    LightEffect _breathingLightEffect;
    LightEffect _blinkingLightEffect;
    LightEffect _colorChangeLightEffect;

    LightEffect _defaultBreathingLightEffect;
    LightEffect _defaultBlinkingLightEffect;
    LightEffect _defaultColorChangeLightEffect;

    ELightMode _mode = ELightMode::None;
    bool _enable = false;

    int _colorSetId = 0;

    const int _delayPartition = 255;
    const int _maxBright = 255;

    static const uint32_t black_ = 0x00000000;
    static const int blackColorSetIndex_ = -1;

    static const std::vector<unsigned int> breathingSpeeds_;
    static const std::vector<unsigned int> blinkingSpeeds_;
};

#endif //ESP32_LIB_NEOPIXEL_H+