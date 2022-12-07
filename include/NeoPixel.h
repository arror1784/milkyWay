#ifndef ESP32_LIB_NEOPIXEL_H
#define ESP32_LIB_NEOPIXEL_H

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#include "Util.h"
#include "SDUtil.h"

enum class EBreathingStatus {
    UP, DOWN
};

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

class NeoPixel {
public:
    NeoPixel(uint16_t nLed, int16_t pin, neoPixelType type);

    void on(uint8_t brightness = 255);

    void off();

    void lowerBrightness();

    void increaseBrightness();

    bool isOn() const;

    EBreathingStatus getBreathingStatus() const;

    uint8_t getMaxBrightness() const;

    uint8_t getBrightness() const;

    void setColorSet(const ColorSet &colorSet);

    void setBreathingStatus(EBreathingStatus status);

private:
    void updatePixelColor();

    EBreathingStatus _breathingStatus = EBreathingStatus::UP;

    ColorSet _colorSet;
    Adafruit_NeoPixel _strip;

    int16_t pin_ = 32;
    int ledCount_ = 100;
    const uint8_t maxBright_ = 255;
};

#endif //ESP32_LIB_NEOPIXEL_H