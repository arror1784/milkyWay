#include "Neopixel.h"

Neopixel::Neopixel(int nLed, int pin, neoPixelType type,boolean isTask) : _nLed(nLed), _pin(pin), _strip(nLed, pin, type),_isTask(isTask)
{

}

void Neopixel::begin()
{
    _strip.begin();
    _strip.show();
}

void Neopixel::plotColorSet(long colorSetIndex, uint8_t br) {
    LightEffect lightEffect = currentLightEffect();

    uint32_t c;
    uint8_t r, g, b;
    if (colorSetIndex < -1 || colorSetIndex >= lightEffect.colorSets.size()) return;

    for (int i = 0; i < _nLed; i++) {
        if (colorSetIndex == -1) {
            _strip.setPixelColor(i, black_);
        }
        else {
            c = lightEffect.colorSets[colorSetIndex]->colors[i];
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

void Neopixel::plotColorSet(long colorSetIndex) {
    plotColorSet(colorSetIndex, _maxBright);
}

void Neopixel::colorChange(int changes, unsigned time)
{
    LightEffect lightEffect = currentLightEffect();

    int prevcolorSetIndex = -1;
    int currentcolorSetIndex;
    
    for (int i = 0; i < changes; i++) {
        if (prevcolorSetIndex == -1) {
            currentcolorSetIndex = random(lightEffect.colorSets.size());
        }
        else {
            currentcolorSetIndex = random(lightEffect.colorSets.size() - 1);
            if (currentcolorSetIndex >= prevcolorSetIndex) currentcolorSetIndex++;
        }

        for (int j = 0; j < _delayPartition / 2; j++) {
            plotColorSet(currentcolorSetIndex, _maxBright * j * 2 / _delayPartition);
            delelOrTaskDelay(time / _delayPartition);
        }

        for (int j = _delayPartition / 2; j < _delayPartition; j++) {
            plotColorSet(currentcolorSetIndex, _maxBright * (_delayPartition - j - 1) * 2 / _delayPartition);
            delelOrTaskDelay(time / _delayPartition);
        }

        prevcolorSetIndex = currentcolorSetIndex;
    }
}

void Neopixel::dim(int dims, unsigned int time)
{
    LightEffect lightEffect = currentLightEffect();
    int colorSetIndex = random(lightEffect.colorSets.size());

    for (int i = 0; i < dims; i++) {

        for (int j = 0; j < _delayPartition / 2; j++) {
            plotColorSet(colorSetIndex, _maxBright * j * 2 / _delayPartition);
            delelOrTaskDelay(time / _delayPartition);
        }

        for (int j = _delayPartition / 2; j < _delayPartition; j++) {
            plotColorSet(colorSetIndex, _maxBright * (_delayPartition - j - 1) * 2 / _delayPartition);
            delelOrTaskDelay(time / _delayPartition);
        }

        delelOrTaskDelay(500);
    }
}

void Neopixel::blink(int blinks, unsigned int time)
{
    LightEffect lightEffect = currentLightEffect();
    int colorSetIndex = random(lightEffect.colorSets.size());

    for (int i = 0; i < blinks; i++) {
        plotColorSet(colorSetIndex);
        delelOrTaskDelay(time);

        plotColorSet(blackColorSetIndex_);
        delelOrTaskDelay(time);
    }
}

void Neopixel::initData(const JsonObject &registerDeviceRes) {
    setLightEffects(registerDeviceRes["lightEffects"]);
    _mode = Util::stringToELightMode(registerDeviceRes["userMode"]["lightMode"]);
}

void Neopixel::setLightEffects(const JsonArray &jsonLightEffects) {
    for (JsonObject jsonLightEffect: jsonLightEffects) {
        ELightMode mode = Util::stringToELightMode(jsonLightEffect["mode"]);

        LightEffect *lightEffect;

        if (mode == Breathing) lightEffect = &_breathingLightEffect;
        if (mode == Blinking) lightEffect = &_blinkingLightEffect;
        if (mode == ColorChange) lightEffect = &_colorChangeLightEffect;

        lightEffect->id = jsonLightEffect["id"];

        for (JsonObject jsonColorSet: JsonArray(jsonLightEffect["colors"])) {
            auto colorSet = new ColorSet();

            colorSet->id = jsonColorSet["id"];

            for (String color: JsonArray(jsonColorSet["colors"])) {
                colorSet->colors.push_back(Util::stringToRGBW(color));
            }
            lightEffect->colorSets.push_back(colorSet);
        }

        lightEffect->mode = Util::stringToELightMode(jsonLightEffect["mode"]);
        lightEffect->isRandomColor = jsonLightEffect["isRandomColor"];
        lightEffect->speed = jsonLightEffect["speed"];
        lightEffect->isRandomSpeed = jsonLightEffect["isRandomSpeed"];
    }
}

const LightEffect &Neopixel::currentLightEffect() {
    if (_mode == Breathing) return _breathingLightEffect;
    if (_mode == Blinking) return _blinkingLightEffect;
    if (_mode == ColorChange) return _colorChangeLightEffect;
}

void Neopixel::delelOrTaskDelay(uint32_t time) {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    if (_isTask) {
        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(time));
    }
    else {
        delay(time);
    }
}
