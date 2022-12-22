#include "NeoPixelTask.h"

NeoPixelTask::NeoPixelTask() : _neoPixel(LED_LENGTH, LED_PIN, NEO_GRBW | NEO_KHZ800), _msgQueue(30), _syncMsgQueue(30) {
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

    _currentLightEffect = &_defaultColorChangeLightEffect;

    String fileStr = SDUtil::readFile(SDUtil::defaultColorSetsPath_);

    DynamicJsonDocument doc = DynamicJsonDocument(fileStr.length() * 2);
    deserializeJson(doc, fileStr);

    _neoPixel.setLedCount(doc["ledCount"]);

    for (auto jsonColorSet: JsonArray(doc["colors"])) {
        ColorSet colorSet;
        for (auto jsonColor: JsonArray(jsonColorSet["colors"])) {
            colorSet.colors.push_back(Util::stringToRGBW(String(jsonColor)));
        }

        _defaultBreathingLightEffect.colorSets.push_back(colorSet);
        _defaultBlinkingLightEffect.colorSets.push_back(colorSet);
        _defaultColorChangeLightEffect.colorSets.push_back(colorSet);
    }

    _neoPixel.off();
}

void NeoPixelTask::sendMsg(NeoPixelMsgData *dataN) {
    _msgQueue.send(dataN);
}

void NeoPixelTask::sendSyncMsg(NeoPixelMsgData *dataN) {
    _syncMsgQueue.send(dataN);
}

bool NeoPixelTask::updateCustomLightEffect(const LightEffect &lightEffect) {
    Serial.println("NeoPixelTask::updateCustomLightEffect lightEffect id : " + String(lightEffect.id));
    bool isChanged = false;
    ELightMode mode = lightEffect.mode;

    LightEffect *oldLightEffect;

    if (mode == ELightMode::Breathing) oldLightEffect = &_breathingLightEffect;
    if (mode == ELightMode::Blinking) oldLightEffect = &_blinkingLightEffect;
    if (mode == ELightMode::ColorChange) oldLightEffect = &_colorChangeLightEffect;

    if (lightEffect.isRandomSpeed != oldLightEffect->isRandomSpeed) isChanged = true;
    else if (lightEffect.isRandomColor != oldLightEffect->isRandomColor) isChanged = true;
    else if (lightEffect.speed != oldLightEffect->speed) isChanged = true;
    else if (lightEffect.colorSets.size() != oldLightEffect->colorSets.size()) isChanged = true;
    else {
        for (int i = 0; i < lightEffect.colorSets.size(); i++) {
            const ColorSet &colorSet = lightEffect.colorSets[i];
            const ColorSet &oldColorSet = oldLightEffect->colorSets[i];
            for (int j = 0; j < colorSet.colors.size(); j++) {
                if (colorSet.colors[j] != oldColorSet.colors[j]) {
                    isChanged = true;
                    break;
                }
                if (isChanged) {
                    break;
                }
            }
        }
    }

    if (mode == ELightMode::Breathing) {
        if (lightEffect.id == 0)
            _breathingLightEffect = _defaultBreathingLightEffect;
        else
            _breathingLightEffect = lightEffect;
    }
    if (mode == ELightMode::Blinking) {
        if (lightEffect.id == 0)
            _blinkingLightEffect = _defaultBlinkingLightEffect;
        else
            _blinkingLightEffect = lightEffect;
    }
    if (mode == ELightMode::ColorChange) {
        if (lightEffect.id == 0)
            _colorChangeLightEffect = _defaultColorChangeLightEffect;
        else
            _colorChangeLightEffect = lightEffect;
    }

    Serial.println("NeoPixelTask::updateCustomLightEffect isChanged : " + String(isChanged));
    Serial.println("NeoPixelTask::updateCustomLightEffect mode : " + String((int) mode));
    Serial.println(
        "NeoPixelTask::updateCustomLightEffect _currentLightEffect->mode : " + String((int) _currentLightEffect->mode));

    return isChanged && mode == _currentLightEffect->mode;
}

// 현재 조명효과를 모드를 기반으로 바꾼다.
bool NeoPixelTask::setCurrentLightEffect(ELightMode mode) {
    ELightMode oldLightMode;
    switch (mode) {
        case ELightMode::None:
            setCurrentLightEffect(_currentLightEffect->mode);
            return true;
        case ELightMode::Breathing:
            oldLightMode = _currentLightEffect->mode;
            _currentLightEffect = _breathingLightEffect.id == 0
                                  ? &_defaultBreathingLightEffect : &_breathingLightEffect;
            Serial.println(
                "NeoPixelTask::setCurrentLightEffect : _breathingLightEffect.id : " + String(_breathingLightEffect.id));
            Serial.println("NeoPixelTask::setCurrentLightEffect : oldLightMode : " + String((int) oldLightMode));
            Serial.println("NeoPixelTask::setCurrentLightEffect : _currentLightEffect->mode : " +
                           String((int) _currentLightEffect->mode));
            return oldLightMode != _currentLightEffect->mode;
        case ELightMode::Blinking:
            oldLightMode = _currentLightEffect->mode;
            _currentLightEffect = _blinkingLightEffect.id == 0
                                  ? &_defaultBlinkingLightEffect : &_blinkingLightEffect;
            Serial.println(
                "NeoPixelTask::setCurrentLightEffect : _blinkingLightEffect.id : " + String(_blinkingLightEffect.id));
            Serial.println("NeoPixelTask::setCurrentLightEffect : oldLightMode : " + String((int) oldLightMode));
            Serial.println("NeoPixelTask::setCurrentLightEffect : _currentLightEffect->mode : " +
                           String((int) _currentLightEffect->mode));
            return oldLightMode != _currentLightEffect->mode;
        case ELightMode::ColorChange:
            oldLightMode = _currentLightEffect->mode;
            _currentLightEffect = _colorChangeLightEffect.id == 0
                                  ? &_defaultColorChangeLightEffect : &_colorChangeLightEffect;
            Serial.println("NeoPixelTask::setCurrentLightEffect : _colorChangeLightEffect.id : " +
                           String(_colorChangeLightEffect.id));
            Serial.println("NeoPixelTask::setCurrentLightEffect : oldLightMode : " + String((int) oldLightMode));
            Serial.println("NeoPixelTask::setCurrentLightEffect : _currentLightEffect->mode : " +
                           String((int) _currentLightEffect->mode));
            return oldLightMode != _currentLightEffect->mode;
        default:
            if (_lightEffectIndexes.empty()) {
                for (int i = 0; i < _lightEffectModeCount; i++) {
                    _lightEffectIndexes.push_back(i);
                }
            }
            long randomValue = random((long) _lightEffectIndexes.size());
            int randomColorSetIndex = _lightEffectIndexes[randomValue];
            _lightEffectIndexes.erase(_lightEffectIndexes.begin() + randomValue);

            return setCurrentLightEffect((ELightMode) randomColorSetIndex);
    }
}

void NeoPixelTask::task() {
    NeoPixelMsgData *msg = _msgQueue.recv();

    if (msg != nullptr) {
        if (msg->events == ENeoPixelMQEvent::UPDATE_EFFECT) {
            bool status = updateCustomLightEffect(msg->lightEffect);
            Serial.println("NeoPixelTask::task : ENeoPixelMQEvent::UPDATE_EFFECT : " + String(status));
            if (status) {
                refreshColorSet(true);

                applyEvent();
            }
        }
        else if (msg->events == ENeoPixelMQEvent::UPDATE_MODE) {
            Serial.println("NeoPixelTask::task : ENeoPixelMQEvent::UPDATE_MODE");
            _mode = msg->mode;
            bool status = setCurrentLightEffect(msg->mode);
            Serial.println("NeoPixelTask::task : setCurrentLightEffect : " + String(status));
            if (status) {
                refreshColorSet(true);

                applyEvent();
            }
        }
        else if (msg->events == ENeoPixelMQEvent::UPDATE_ENABLE) {
            Serial.println("NeoPixelTask::task : ENeoPixelMQEvent::UPDATE_ENABLE : " + String(msg->enable));
            _isEnabled = msg->enable;
            _count = msg->count;
            if (_isEnabled) {
                setCurrentLightEffect(_mode);

                refreshColorSet(true);

                applyEvent();
            }
            else {
                reset();
                _nextTick = 0xFFFFFFFF;
            }
        }
        delete msg;
    }

    msg = _syncMsgQueue.recv();

    if (msg != nullptr) {
        if (msg->events == ENeoPixelMQEvent::UPDATE_SYNC) {
            _isSyncMode = msg->enable;
            _sync = msg->sync;
            _speed = 24;
        }
        delete msg;
    }

    if (millis() >= _nextTick) {
        ticked();
    }
}

void NeoPixelTask::applyEvent() {
    Serial.println("NeoPixelTask::applyEvent : _isEnabled : " + String(_isEnabled));
    if (_isEnabled && !getCurrentColorSet().empty()) {
        refreshSpeed();
        refreshNextTick();

        Serial.println("NeoPixelTask::applyEvent : _speed : " + String(_speed));
    }
}

void NeoPixelTask::ticked() {
    bool isCycleFinished = false;

    if (_isSyncMode) {
        sync();
    }
    else if (_currentLightEffect->mode == ELightMode::Blinking) {
        isCycleFinished = blink();
    }
    else if (_currentLightEffect->mode == ELightMode::Breathing ||
             _currentLightEffect->mode == ELightMode::ColorChange) {
        isCycleFinished = breath();
    }

    if (isCycleFinished) finishCycle();
}

bool NeoPixelTask::breath() {
    EBreathingStatus currentBreathingStatus = _neoPixel.getBreathingStatus();
    if (currentBreathingStatus == EBreathingStatus::UP) {
        _neoPixel.increaseBrightness();
    }
    else {
        _neoPixel.lowerBrightness();
    }

    auto brightness = _neoPixel.getBrightness();
    auto maxBrightness = _neoPixel.getMaxBrightness();

    if (brightness == 0) {
        _neoPixel.setBreathingStatus(EBreathingStatus::UP);

        return true;
    }
    if (brightness == maxBrightness) {
        _neoPixel.setBreathingStatus(EBreathingStatus::DOWN);
    }
    return false;
}

bool NeoPixelTask::blink() {
    if (_neoPixel.isOn()) {
        _neoPixel.off();

        Serial.println("NeoPixelTask::blink : _speed : " + String(_speed));
        return true;
    }
    _neoPixel.on();

    refreshNextTick();

    return false;
}

void NeoPixelTask::sync() {
    _neoPixel.on(_sync);

    refreshNextTick();
}

void NeoPixelTask::finishCycle() {
    if (_mode == ELightMode::Mixed && _count < 0) {
        setCurrentLightEffect(_mode);

        refreshColorSet(true);
    }
    else if (_currentLightEffect->mode == ELightMode::ColorChange) {
        refreshColorSet(true);
    }
    refreshSpeed();
    refreshNextTick();

    Serial.println("NeoPixelTask::finishCycle _speed : " + String(_speed));

    auto dataP = new PingPongMsgData();

    dataP->enable = true;
    dataP->events = EPingPongMQEvent::FINISH_NEO_PIXEL;

    _isEnabled = false;

    PingPongMsgQueue::getInstance().send(dataP);
}

void NeoPixelTask::reset() {
    _neoPixel.off();
    _neoPixel.setBreathingStatus(EBreathingStatus::UP);
    _nextTick = 0xFFFFFFFF;
}

// 네오픽셀에게 현재 조명효과내에 있는 컬러셋 중 하나로 갱신하여 넘겨준다.
void NeoPixelTask::refreshColorSet(bool shouldResetColorIndexes) {
    const std::vector<ColorSet> &colorSets = getCurrentColorSet();

    if (shouldResetColorIndexes || _colorIndexes.empty()) {
        _colorIndexes.clear();
        for (int i = 0; i < colorSets.size(); i++) {
            _colorIndexes.push_back(i);
        }
    }

    long randomValue = random((long) _colorIndexes.size());
    int randomColorSetIndex = _colorIndexes[randomValue];
    _colorIndexes.erase(_colorIndexes.begin() + randomValue);

    _neoPixel.setColorSet(colorSets[randomColorSetIndex]);

    Serial.println("NeoPixelTask::refreshColorSet : " + String(randomColorSetIndex));
}

// 현재 조명효과의 모드에 따른 스피드를 넣는다.
void NeoPixelTask::refreshSpeed() {
    if (_isSyncMode) {
        _speed = 24;
        return;
    }

    switch (_currentLightEffect->mode) {
        // blinking은 무조건 랜덤
        case ELightMode::Blinking:
            _speed = _blinkingSpeeds[random(_blinkingSpeeds.size())];
            break;
        case ELightMode::Breathing:
        case ELightMode::ColorChange:
            _speed = ((_currentLightEffect->isRandomSpeed
                       ? _breathingSpeeds[random(_breathingSpeeds.size())] : _currentLightEffect->speed)) /
                     _neoPixel.getMaxBrightness();
            break;
        default:
            break;
    }
}

// 현재 스피드를 기반으로 다음 틱 시간을 설정한다.
void NeoPixelTask::refreshNextTick() {
    _nextTick = millis() + _speed;
}

const std::vector<ColorSet> &NeoPixelTask::getCurrentColorSet() {
    Serial.println("NeoPixelTask::getCurrentColorSet: " + String(_currentLightEffect->id));

    if (_currentLightEffect->isRandomColor) {
        if (_currentLightEffect->mode == ELightMode::Breathing) {
            Serial.println("NeoPixelTask::getCurrentColorSet: _defaultBreathingLightEffect");
            return _defaultBreathingLightEffect.colorSets;
        }
        if (_currentLightEffect->mode == ELightMode::Blinking) {
            Serial.println("NeoPixelTask::getCurrentColorSet: _defaultBlinkingLightEffect");
            return _defaultBlinkingLightEffect.colorSets;
        }
        if (_currentLightEffect->mode == ELightMode::ColorChange) {
            Serial.println("NeoPixelTask::getCurrentColorSet: _defaultColorChangeLightEffect");
            return _defaultColorChangeLightEffect.colorSets;
        }
    }

    Serial.println("NeoPixelTask::getCurrentColorSet: _currentLightEffect");
    return _currentLightEffect->colorSets;
}

ELightMode NeoPixelTask::getCurrentMode() {
    return _mode;
}
