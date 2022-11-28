#include "Neopixel.h"

Neopixel::Neopixel(int nLed, int pin, neoPixelType type) : _nLed(nLed), _pin(pin), _strip(nLed, pin, type)
{

}

void Neopixel::begin()
{
    _strip.begin();
    _strip.show();
}

void Neopixel::plotColorSet(LightEffect lightEffect,int colorSetIndex, uint8_t br) {

    uint32_t c;
    uint8_t r, g, b;
    
    int size = lightEffect.colorSets.size();

    if (colorSetIndex < -1 || colorSetIndex >= size) 
        return;

    for (int i = 0; i < _nLed; i++) {

        if (colorSetIndex == -1) {
            _strip.setPixelColor(i, black_);
        }
        else {
            c = lightEffect.colorSets[colorSetIndex].colors[i];
            r = (c & 0x00FF0000) >> 16;
            g = (c & 0x0000FF00) >> 8;
            b = (c & 0x000000FF);
            c = (r * br / _maxBright) << 16 |
                (g * br / _maxBright) << 8 |
                (b * br / _maxBright);
            _strip.setPixelColor(i, c);
        }
    }
    _strip.show();
}

void Neopixel::plotColorSet(LightEffect lightEffect,int colorSetIndex) {
    plotColorSet(lightEffect,colorSetIndex, _maxBright);
}

void Neopixel::colorChange(int changes, unsigned time)
{

    int prevcolorSetIndex = -1;
    int currentcolorSetIndex;
    
    for (int i = 0; i < changes; i++) {

        while(1){
            currentcolorSetIndex = random(_colorChangeLightEffect.colorSets.size());
            if(prevcolorSetIndex != currentcolorSetIndex)
                break;
            
        }
        for (int j = 0; j < _delayPartition / 2; j++) {
            plotColorSet(_colorChangeLightEffect,currentcolorSetIndex, _maxBright * j * 2 / _delayPartition);
            Util::taskDelay(time / _delayPartition);
        }

        for (int j = _delayPartition / 2; j < _delayPartition; j++) {
            plotColorSet(_colorChangeLightEffect,currentcolorSetIndex, _maxBright * (_delayPartition - j - 1) * 2 / _delayPartition);
            Util::taskDelay(time / _delayPartition);
        }

        prevcolorSetIndex = currentcolorSetIndex;
    }
}

void Neopixel::dim(int dims, unsigned int time)
{
    for (int i = 0; i < dims; i++) {

        for (int j = 0; j < _delayPartition / 2; j++) {
            plotColorSet(_breathingLightEffect,_colorPresetIndex, _maxBright * j * 2 / _delayPartition);
            Util::taskDelay(time / _delayPartition);
        }

        for (int j = _delayPartition / 2; j < _delayPartition; j++) {
            plotColorSet(_breathingLightEffect,_colorPresetIndex, _maxBright * (_delayPartition - j - 1) * 2 / _delayPartition);
            Util::taskDelay(time / _delayPartition);
        }
    }
}

void Neopixel::blink(int blinks, unsigned int time)
{
    for (int i = 0; i < blinks; i++) {
        plotColorSet(_blinkingLightEffect, _colorPresetIndex);
        Util::taskDelay(time);

        plotColorSet(_blinkingLightEffect, -1,0);
        Util::taskDelay(time);
    }
}

void Neopixel::initData(const LightEffect &lightEffect,ELightMode mode) {
    setLightEffects(lightEffect);
    _mode = mode;
}

void Neopixel::setLightEffects(const LightEffect &lightEffects){

    ELightMode mode = lightEffects.mode;

    if (mode == ELightMode::Breathing) _breathingLightEffect = lightEffects;
    if (mode == ELightMode::Blinking) _blinkingLightEffect = lightEffects;
    if (mode == ELightMode::ColorChange) _colorChangeLightEffect = lightEffects;
}

void Neopixel::loop() {
    if(!_enable){
        Util::taskDelay(1000);
        return;
    }
    switch (_mode)
    {
        case ELightMode::Blinking :
            blink(1,_blinkingLightEffect.speed);
            return;
        case ELightMode::Breathing :
            dim(1,_breathingLightEffect.speed);
            return;
        case ELightMode::ColorChange :
            colorChange(1,_colorChangeLightEffect.speed);
            return;
        case ELightMode::Mixed :
            if(_mixCount == _MaxMixCount){
                _mixCount = 0;
                while(1){
                    int i = random(3);
                    if(i != _mixMode){
                        _mixMode = i;
                        break;
                    }
                } 
            }
            _mixCount++;
            if(_mixMode == 0) blink(1,_blinkingLightEffect.speed);
            if(_mixMode == 1) dim(1,_breathingLightEffect.speed);
            if(_mixMode == 2) colorChange(1,_colorChangeLightEffect.speed);
            return;
        case ELightMode::Sync :
            return;
        default:
            return;
    }
}

void Neopixel::changeMode(ELightMode mode) {
    _mode = mode;
    if (mode == ELightMode::Breathing) _colorPresetIndex = random(_breathingLightEffect.colorSets.size());
    if (mode == ELightMode::Blinking) _colorPresetIndex = random(_breathingLightEffect.colorSets.size());
    // if (mode == ELightMode::ColorChange) return _colorChangeLightEffect;
}
