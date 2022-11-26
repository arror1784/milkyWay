#include "Neopixel.h"

Neopixel::Neopixel(int nLed, int pin, neoPixelType type,boolean isTask) : _nLed(nLed), _pin(pin), _strip(nLed, pin, type),_isTask(isTask)
{

}

void Neopixel::begin()
{
    _strip.begin();
    _strip.show();
}

void Neopixel::plotPattern(int patternNum, uint8_t br)
{
    uint32_t c;
    uint8_t r, g, b;
    if (patternNum < -1 || patternNum >= getPresetLength())
        return;
    for (int i = 0; i < _nLed; i++) {
        if (patternNum == -1) {
            _strip.setPixelColor(i, BLACK);
        } else {
            c = _colorPreSet[patternNum][i];
            r = (c & 0x00FF0000) >> 16;
            g = (c & 0x0000FF00) >> 8;
            b = (c & 0x000000FF);
            c = (r * br / MAX_BR) << 16 |
                (g * br / MAX_BR) << 8 |
                (b * br / MAX_BR);
            _strip.setPixelColor(i, c);
        }
    }
    _strip.show();
}

void Neopixel::colorChange(int changes, unsigned time)
{
    int prevPatternIndex = -1, currentPatternIndex;
    uint32_t c;
    uint8_t r, g, b;
    
    for (int i = 0; i < changes; i++) {
        if (prevPatternIndex == -1) {
            currentPatternIndex = random(getPresetLength());
        } else {
            currentPatternIndex = random(getPresetLength() - 1);
            if (currentPatternIndex >= prevPatternIndex) currentPatternIndex++;
        }
        
        for (int j = 0; j < DELAY_PARTITION / 2; j++) {
            plotPattern(currentPatternIndex, MAX_BR * j * 2 / DELAY_PARTITION);
            delelOrTaskDelay(time / DELAY_PARTITION);
        }
        
        for (int j = DELAY_PARTITION / 2; j < DELAY_PARTITION; j++) {
            plotPattern(currentPatternIndex, MAX_BR * (DELAY_PARTITION - j - 1) * 2 / DELAY_PARTITION);
            delelOrTaskDelay(time / DELAY_PARTITION);
        }

        prevPatternIndex = currentPatternIndex;
    }
}

void Neopixel::dim(int dims, unsigned int time)
{
    int patternIndex = random(getPresetLength());
    uint32_t c;
    uint8_t r, g, b;

    for (int i = 0; i < dims; i++) {

        for (int j = 0; j < DELAY_PARTITION / 2; j++) {
            plotPattern(patternIndex, MAX_BR * j * 2 / DELAY_PARTITION);
            delelOrTaskDelay(time / DELAY_PARTITION);
        }

        for (int j = DELAY_PARTITION / 2; j < DELAY_PARTITION; j++) {
            plotPattern(patternIndex, MAX_BR * (DELAY_PARTITION - j - 1) * 2 / DELAY_PARTITION);
            delelOrTaskDelay(time / DELAY_PARTITION);
        }

        delelOrTaskDelay(500);
    }
}

void Neopixel::blink(int blinks, unsigned int time)
{
    int patternIndex = random(getPresetLength());

    for (int i = 0; i < blinks; i++) {
        plotPattern(patternIndex);
        delelOrTaskDelay(time);

        plotPattern(PATTERN_INDEX_BLACK);
        delelOrTaskDelay(time);
    }
}

void Neopixel::pushColorPreset(std::vector<uint32_t>& preset){
    _colorPreSet.push_back(preset);
}
void Neopixel::clearColorPreset(){
    _colorPreSet.clear();
}

void Neopixel::delelOrTaskDelay(uint32_t time){
    TickType_t xLastWakeTime = xTaskGetTickCount();

    if(_isTask)
        xTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(time));
    else
        delay(time);
}