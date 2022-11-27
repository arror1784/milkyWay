#ifndef ESP32_LIB_NEOPIXEL_H
#define ESP32_LIB_NEOPIXEL_H

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>

#include "Util.h"

class ColorSet {
public:
    long id;
    std::vector<uint32_t> colors;
};

class LightEffect {
public:
    long id;
    std::vector<ColorSet> colorSets;
    ELightMode mode = ELightMode::Blinking;
    bool isRandomColor;
    long speed;
    bool isRandomSpeed;
};

class Neopixel {
public:
    Neopixel(int nLed, int pin, neoPixelType type,boolean isTask=false);
    void begin();
    void plotColorSet(LightEffect lightEffect,int colorSetIndex, uint8_t br);
    void plotColorSet(LightEffect lightEffect,int colorSetIndex);
    void colorChange(int changes, unsigned time);
    void dim(int dims, unsigned int time);
    void blink(int blinks, unsigned int time);

    void initData(const LightEffect &lightEffect,ELightMode mode);
    void setLightEffects(const LightEffect &lightEffects);
    void changeMode(ELightMode mode);
    const LightEffect &currentLightEffect();
    
    void loop();

    void setEnable(bool enable){_enable = enable;};

private:
    void delelOrTaskDelay(uint32_t time);

    Adafruit_NeoPixel _strip;
    int _pin;

    int _selectColorPreset;
    int _nLed;

    int _mixCount = 0;
    int _mixMode = 0;
    const int _MaxMixCount = 4;
    int _colorPresetIndex = 0;

    boolean _isTask = false;

    LightEffect _breathingLightEffect;
    LightEffect _blinkingLightEffect;
    LightEffect _colorChangeLightEffect;

    ELightMode _mode = ELightMode::None;
    bool _enable = true;

    int _colorSetId = 0;

    const int _delayPartition = 255;
    const int _maxBright = 255;

    static const uint32_t black_ = 0x00000000;
    static const int blackColorSetIndex_ = -1;
};

#endif //ESP32_LIB_NEOPIXEL_H+