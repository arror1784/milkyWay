#include "NeoPixel.h"

const std::vector<unsigned int>  Neopixel::breathingSpeeds_{
    1500, 2500, 4000, 5000, 6000, 8000, 10000, 12000, 15000
};

const std::vector<unsigned int>  Neopixel::blinkingSpeeds_{
    500, 1000, 1500, 2000, 2500, 3000, 5000
};

Neopixel::Neopixel(int nLed, int pin, neoPixelType type) : _nLed(nLed), _pin(pin), _strip(nLed, pin, type) {
    _defaultBreathingLightEffect = {
        .id = 0,
        .colorSets{},
        .mode = ELightMode::Breathing,
        .isRandomColor = true,
        .speed = 5500,
        .isRandomSpeed = true
    };

    _defaultBlinkingLightEffect = {
        .id = 0,
        .colorSets{},
        .mode = ELightMode::Blinking,
        .isRandomColor = true,
        .speed = 5500,
        .isRandomSpeed = true
    };

    _defaultColorChangeLightEffect = {
        .id = 0,
        .colorSets{},
        .mode = ELightMode::ColorChange,
        .isRandomColor = true,
        .speed = 5500,
        .isRandomSpeed = true
    };

    String fileStr = SDUtil::readFile(SDUtil::defaultColorSetsPath_);

    DynamicJsonDocument doc = DynamicJsonDocument(fileStr.length() * 2);
    deserializeJson(doc, fileStr);

    for (auto jsonColorSet: JsonArray(doc["colors"])) {
        ColorSet colorSet;
        for (auto jsonColor: JsonArray(jsonColorSet["colors"])) {
            colorSet.colors.push_back(Util::stringToRGBW(String(jsonColor)));
        }

        _defaultBreathingLightEffect.colorSets.push_back(colorSet);
        _defaultBlinkingLightEffect.colorSets.push_back(colorSet);
        _defaultColorChangeLightEffect.colorSets.push_back(colorSet);
    }
}

void Neopixel::begin() {
    _strip.begin();
    _strip.show();
}

void Neopixel::plotColorSet(const LightEffect &lightEffect, int colorSetIndex, uint8_t br) {

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

void Neopixel::plotColorSet(const LightEffect &lightEffect, int colorSetIndex) {
    plotColorSet(lightEffect, colorSetIndex, _maxBright);
}

void Neopixel::sync(uint8_t per) {
    const LightEffect &lightEffect = getLightEffect(_mode);

    plotColorSet(lightEffect, _colorPresetIndex, per);
}

void Neopixel::colorChange(int changes) {
    const LightEffect &lightEffect = getLightEffect(ELightMode::ColorChange);

    unsigned int time = lightEffect.speed;

    int prevcolorSetIndex = -1;
    int currentcolorSetIndex;

    for (int i = 0; i < changes; i++) {

        while (1) {
            currentcolorSetIndex = random(lightEffect.colorSets.size());
            if (prevcolorSetIndex != currentcolorSetIndex)
                break;

        }
        for (int j = 0; j < _delayPartition / 2; j++) {
            plotColorSet(lightEffect, currentcolorSetIndex, _maxBright * j * 2 / _delayPartition);
            Util::taskDelay(time / _delayPartition);
        }

        for (int j = _delayPartition / 2; j < _delayPartition; j++) {
            plotColorSet(lightEffect, currentcolorSetIndex,
                         _maxBright * (_delayPartition - j - 1) * 2 / _delayPartition);
            Util::taskDelay(time / _delayPartition);
        }

        prevcolorSetIndex = currentcolorSetIndex;
    }
}

void Neopixel::dim(int dims) {
    const LightEffect &lightEffect = getLightEffect(ELightMode::Breathing);

    unsigned int time =
        lightEffect.isRandomSpeed
        ? breathingSpeeds_[random(breathingSpeeds_.size())]
        : lightEffect.speed;

    for (int i = 0; i < dims; i++) {

        for (int j = 0; j < _delayPartition / 2; j++) {
            plotColorSet(lightEffect, _colorPresetIndex, _maxBright * j * 2 / _delayPartition);
            Util::taskDelay(time / _delayPartition);
        }

        for (int j = _delayPartition / 2; j < _delayPartition; j++) {
            plotColorSet(lightEffect, _colorPresetIndex,
                         _maxBright * (_delayPartition - j - 1) * 2 / _delayPartition);
            Util::taskDelay(time / _delayPartition);
        }
    }
}

void Neopixel::blink(int blinks) {
    const LightEffect &lightEffect = getLightEffect(ELightMode::Blinking);

    unsigned int time =
        lightEffect.isRandomSpeed
        ? blinkingSpeeds_[random(blinkingSpeeds_.size())]
        : lightEffect.speed;


    for (int i = 0; i < blinks; i++) {
        plotColorSet(lightEffect, _colorPresetIndex);
        Util::taskDelay(time);

        plotColorSet(lightEffect, -1, 0);
        Util::taskDelay(time);
    }
}

void Neopixel::setLightEffect(const LightEffect &lightEffect) {
    ELightMode mode = lightEffect.mode;

    if (mode == ELightMode::Breathing) _breathingLightEffect = lightEffect;
    if (mode == ELightMode::Blinking) _blinkingLightEffect = lightEffect;
    if (mode == ELightMode::ColorChange) _colorChangeLightEffect = lightEffect;
}

void Neopixel::loop() {
    if (!_enable) {
        Util::taskDelay(10);
        return;
    }

    switch (_mode) {
        case ELightMode::Blinking :
            blink(1);
            return;
        case ELightMode::Breathing :
            dim(1);
            return;
        case ELightMode::ColorChange :
            colorChange(1);
            return;
        case ELightMode::Mixed :
            if (_mixCount == _MaxMixCount) {
                _mixCount = 0;
                while (1) {
                    int i = random(3);
                    if (i != _mixMode) {
                        _mixMode = i;
                        break;
                    }
                }
            }
            _mixCount++;
            if (_mixMode == 0) blink(1);
            if (_mixMode == 1) dim(1);
            if (_mixMode == 2) colorChange(1);
            return;
        default:
            return;
    }
}

void Neopixel::changeMode(ELightMode mode) {
    _mode = mode;
    if (mode == ELightMode::Breathing) _colorPresetIndex = random(_breathingLightEffect.colorSets.size());
    if (mode == ELightMode::Blinking) _colorPresetIndex = random(_breathingLightEffect.colorSets.size());
}

const LightEffect &Neopixel::getLightEffect(ELightMode mode) {
    if (_mode == ELightMode::Breathing) {
        return _breathingLightEffect.id == 0
               ? _defaultBreathingLightEffect : _breathingLightEffect;
    }
    else if (_mode == ELightMode::Blinking) {
        return _blinkingLightEffect.id == 0
               ? _defaultBlinkingLightEffect : _blinkingLightEffect;
    }
    else if (_mode == ELightMode::ColorChange) {
        return _colorChangeLightEffect.id == 0
               ? _defaultColorChangeLightEffect
               : _colorChangeLightEffect;
    }

    auto randomValue = random(3);

    if (randomValue == 0) return getLightEffect(ELightMode::Breathing);
    if (randomValue == 1) return getLightEffect(ELightMode::Blinking);
    return getLightEffect(ELightMode::ColorChange);
}
