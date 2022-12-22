#include "NeoPixel.h"

NeoPixel::NeoPixel(uint16_t ledCount, int16_t pin, neoPixelType type) : _strip(ledCount, pin, type) {
    _strip.begin();
    _strip.setBrightness(0);
}

void NeoPixel::on(uint8_t brightness) {
    _strip.setBrightness(brightness);
    updatePixelColor();
    _strip.show();
}

void NeoPixel::off() {
    _strip.setBrightness(0);
    _strip.clear();
    _strip.show();
}

void NeoPixel::lowerBrightness() {
    auto brightness = _strip.getBrightness() - 1;
    _strip.setBrightness(brightness);
    updatePixelColor();
    _strip.show();
}

void NeoPixel::increaseBrightness() {
    auto brightness = _strip.getBrightness() + 1;
    _strip.setBrightness(brightness);
    updatePixelColor();
    _strip.show();
}

bool NeoPixel::isOn() const {
    return _strip.getBrightness() == _maxBright;
}

void NeoPixel::updatePixelColor() {
    for (int i = 0; i < _colorSet.colors.size(); i++) {
        _strip.setPixelColor(i, _colorSet.colors[i]);
    }

    for (int i = (int) _colorSet.colors.size(); i < _ledCount; i++) {
        _strip.setPixelColor(i, _colorSet.colors[_colorSet.colors.size() - 1]);
    }
}

EBreathingStatus NeoPixel::getBreathingStatus() const {
    return _breathingStatus;
}

uint8_t NeoPixel::getMaxBrightness() const {
    return _maxBright;
}

uint8_t NeoPixel::getBrightness() const {
    return _strip.getBrightness();
}

void NeoPixel::setColorSet(const ColorSet &colorSet) {
    _colorSet = colorSet;
}

void NeoPixel::setBreathingStatus(EBreathingStatus status) {
    _breathingStatus = status;
}

void NeoPixel::setLedCount(int ledCount) {
    _ledCount = ledCount;
}

int NeoPixel::getLedCount() {
    return _ledCount;
}
