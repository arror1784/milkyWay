#include "NeoPixel.h"

int16_t NeoPixel::pin_ = 32;

int NeoPixel::ledCount_ = 20;

const int NeoPixel::maxBright_ = 10;

NeoPixel::NeoPixel(uint16_t ledCount, int16_t pin) : _strip(ledCount, pin, NEO_GRBW) {
  _strip.begin();
  _strip.setBrightness(0);
}

void NeoPixel::on() {
  _strip.setBrightness(maxBright_);
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

  if (brightness == 0) {
    _dimmingStatus = EDimmingStatus::UP;
    if (_isColorChange) {
      setRandomColorSetId();
    }
  }
  _strip.show();
}

void NeoPixel::increaseBrightness() {
  auto brightness = _strip.getBrightness() + 1;
  _strip.setBrightness(brightness);
  updatePixelColor();

  if (brightness == maxBright_) {
    _dimmingStatus = EDimmingStatus::DOWN;
  }
  _strip.show();
}

bool NeoPixel::isOn() const {
  return _strip.getBrightness() == maxBright_;
}

bool NeoPixel::isCurrentLightEffectValid() {
  return _lightEffects[_lightEffectId] != nullptr;
}

const LightEffect &NeoPixel::getCurrentLightEffect() {
  return *_lightEffects[_lightEffectId];
}

EDimmingStatus NeoPixel::getDimmingStatus() const {
  return _dimmingStatus;
}

void NeoPixel::setLightEffectId(ELightMode mode) {
  for (auto lightEffect: _lightEffects) {
    if (lightEffect.second->mode == mode) {
      _lightEffectId = lightEffect.first;
    }
  }
}

void NeoPixel::setRandomColorSetId() {
  LightEffect lightEffect = getCurrentLightEffect();
  std::vector<long> keys;
  for (auto colorSet : lightEffect.colorSets) {
    keys.push_back(colorSet.first);
  }
  _colorSetId = keys[random(keys.size())];
}

void NeoPixel::setLightEffect(const JsonObject &jsonLightEffect) {
  long id = jsonLightEffect["id"];

  if (_lightEffects[id] == nullptr) return;

  LightEffect *lightEffect = _lightEffects[id];

  lightEffect->id = jsonLightEffect["id"];

  for (JsonObject jsonColorSet: JsonArray(jsonLightEffect["colors"])) {
    auto colorSet = new ColorSet();

    colorSet->id = jsonColorSet["id"];

    for (String color: JsonArray(jsonColorSet["colors"])) {
      colorSet->colors.push_back(Util::stringToRGBW(color));
    }
    lightEffect->colorSets.insert({colorSet->id, colorSet});
  }

  lightEffect->mode = Util::stringToELightMode(jsonLightEffect["mode"]);
  lightEffect->isRandomColor = jsonLightEffect["isRandomColor"];
  lightEffect->speed = jsonLightEffect["speed"];
  lightEffect->isRandomSpeed = jsonLightEffect["isRandomSpeed"];
}

void NeoPixel::setLightEffects(const JsonArray &jsonLightEffects) {
  for (auto lightEffect: _lightEffects) {
    for (auto colorSet: lightEffect.second->colorSets) {
      delete colorSet.second;
    }
    delete lightEffect.second;
  }

  for (JsonObject jsonLightEffect: jsonLightEffects) {
    long id = jsonLightEffect["id"];
    _lightEffects.insert({id, new LightEffect()});
    setLightEffect(jsonLightEffect);
  }
}

void NeoPixel::setIsColorChange(bool isColorChange) {
  NeoPixel::_isColorChange = isColorChange;
}

void NeoPixel::updatePixelColor() {
  LightEffect *lightEffect = _lightEffects[_lightEffectId];
  ColorSet *colorSet = lightEffect->colorSets[_colorSetId];

  for (int i = 0; i < ledCount_; i++) {
    _strip.setPixelColor(i, colorSet->colors[i]);
  }
}

void NeoPixel::setPin(int16_t pin) {
  NeoPixel::pin_ = pin;
}

void NeoPixel::setLedCount(int ledCount) {
  NeoPixel::ledCount_ = ledCount;
}
