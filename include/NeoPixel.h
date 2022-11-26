#ifndef ESP32_LIB_NEOPIXEL_H
#define ESP32_LIB_NEOPIXEL_H

#include <Adafruit_NeoPixel.h>
#include <vector>

#define MAX_LED 50

#define MAX_BR 255
#define DELAY_PARTITION 255

#define BLACK 0x00000000
#define PATTERN_INDEX_BLACK -1

class Neopixel {
public:
    Neopixel(int nLed, int pin, neoPixelType type,boolean isTask=false);
    void begin();
    void plotPattern(int patternNum, uint8_t br = MAX_BR);
    void colorChange(int changes, unsigned time);
    void dim(int dims, unsigned int time);
    void blink(int blinks, unsigned int time);

    void pushColorPreset(std::vector<uint32_t>&);
    void clearColorPreset();

    int getPresetLength(){return _colorPreSet.size();};

private:
    void delelOrTaskDelay(uint32_t time);

private:
    Adafruit_NeoPixel _strip;
    int _pin;
    std::vector<std::vector<uint32_t>> _colorPreSet;

    int _selectColorPreset;
    int _nLed;
    int _nPattern;

    boolean _isTask = false;
};

#endif //ESP32_LIB_NEOPIXEL_H+