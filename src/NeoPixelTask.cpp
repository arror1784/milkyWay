#include "NeoPixelTask.h"

NeoPixelTask::NeoPixelTask() : _neoPixel(LED_LENGTH, LED_PIN, NEO_GRBW | NEO_KHZ800), _shuffleMsgQueue(30),
                               _msgQueue(30), _syncMsgQueue(30) {
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

ShuffleMsgData *NeoPixelTask::getShuffleMsg() {
    return _shuffleMsgQueue.recv();
}

void NeoPixelTask::sendMsg(NeoPixelMsgData *dataN) {
    _msgQueue.send(dataN);
}

void NeoPixelTask::sendSyncMsg(NeoPixelMsgData *dataN) {
    _syncMsgQueue.send(dataN);
}

bool NeoPixelTask::updateCustomLightEffect(const LightEffect &lightEffect) {
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

    if (mode == ELightMode::Breathing) _breathingLightEffect = lightEffect;
    if (mode == ELightMode::Blinking) _blinkingLightEffect = lightEffect;
    if (mode == ELightMode::ColorChange) _colorChangeLightEffect = lightEffect;

    return isChanged;
}

// 현재 조명효과를 모드를 기반으로 바꾼다.
bool NeoPixelTask::setCurrentLightEffect(ELightMode mode) {
    switch (mode) {
        case ELightMode::Breathing:
            _currentLightEffect = _breathingLightEffect.id == 0
                                  ? &_defaultBreathingLightEffect : &_breathingLightEffect;
            return _currentLightEffect->mode != mode;
        case ELightMode::Blinking:
            _currentLightEffect = _blinkingLightEffect.id == 0
                                  ? &_defaultBlinkingLightEffect : &_blinkingLightEffect;
            return _currentLightEffect->mode != mode;
        case ELightMode::ColorChange:
            _currentLightEffect = _colorChangeLightEffect.id == 0
                                  ? &_defaultColorChangeLightEffect : &_colorChangeLightEffect;
            return _currentLightEffect->mode != mode;
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
            if (status && isValidColorSet()) {
                refreshColorSet();
                refreshSpeed();
                refreshNextTick();
            }
        }
        else if (msg->events == ENeoPixelMQEvent::UPDATE_MODE) {
            bool status = setCurrentLightEffect(msg->mode);
            if (status && isValidColorSet()) {
                refreshColorSet();
                refreshSpeed();
                refreshNextTick();
            }
            _mode = msg->mode;
        }
        else if (msg->events == ENeoPixelMQEvent::UPDATE_ENABLE) {
            if (msg->enable) {
                _isShuffle = msg->isShuffle;
                if (_isShuffle) {
                    _count = _oneCycleCount;
                }
                setCurrentLightEffect(_mode);

                if (isValidColorSet()) {
                    refreshColorSet();
                    refreshSpeed();
                    refreshNextTick();
                }
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
        }
        delete msg;
    }

    if (millis() >= _nextTick) {
        ticked();
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

    if (brightness == maxBrightness) {
        _neoPixel.setBreathingStatus(EBreathingStatus::DOWN);
    }
    else if (brightness == 0) {
        _neoPixel.setBreathingStatus(EBreathingStatus::UP);
        if (_currentLightEffect->mode == ELightMode::ColorChange) {
            refreshColorSet();
        }
        refreshSpeed();
        refreshNextTick();
        return true;
    }
    refreshNextTick();
    return false;
}

bool NeoPixelTask::blink() {
    if (_neoPixel.isOn()) {
        _neoPixel.off();
        refreshSpeed();
        refreshNextTick();
        return true;
    }
    _neoPixel.on();

    refreshNextTick();

    return false;
}

void NeoPixelTask::sync() {
    _speed = 24;
    _neoPixel.on(_sync);

    refreshNextTick();
}

void NeoPixelTask::finishCycle() {
    if (_count == -1) {
        if (_mode == ELightMode::Mixed) {
            setCurrentLightEffect(_mode);
            refreshColorSet();
            refreshSpeed();
            refreshNextTick();
        }
    }
    else if (_isShuffle) {
        if (_count > 1) {
            _count -= 1;
        }
        else if (_count == 1) {
            auto dataS = new ShuffleMsgData();

            dataS->enable = true;
            dataS->events = EShuffleSMQEvent::FINISH_NEO_PIXEL;

            _shuffleMsgQueue.send(dataS);

            _count = -1;

            reset();
        }
    }
}

void NeoPixelTask::reset() {
    _neoPixel.off();
    _neoPixel.setBreathingStatus(EBreathingStatus::UP);
    _isShuffle = false;
    _nextTick = 0xFFFFFFFF;
}

// 네오픽셀에게 현재 조명효과내에 있는 컬러셋 중 하나로 갱신하여 넘겨준다.
void NeoPixelTask::refreshColorSet() {
    std::vector<ColorSet> *colorSets = &_currentLightEffect->colorSets;

    if (_currentLightEffect->isRandomColor) {
        if (_currentLightEffect->mode == ELightMode::Breathing) colorSets = &_defaultBreathingLightEffect.colorSets;
        if (_currentLightEffect->mode == ELightMode::Blinking) colorSets = &_defaultBlinkingLightEffect.colorSets;
    }

    if (_colorIndexes.empty()) {
        for (int i = 0; i < colorSets->size(); i++) {
            _colorIndexes.push_back(i);
        }
    }

    long randomValue = random((long) _colorIndexes.size());
    int randomColorSetIndex = _colorIndexes[randomValue];
    _colorIndexes.erase(_colorIndexes.begin() + randomValue);

    _neoPixel.setColorSet(colorSets->operator[](randomColorSetIndex));
}

// 현재 조명효과의 모드에 따른 스피드를 넣는다.
void NeoPixelTask::refreshSpeed() {
    switch (_currentLightEffect->mode) {
        // blinking은 무조건 랜덤
        case ELightMode::Blinking:
            _speed = _blinkingSpeeds[random(_blinkingSpeeds.size())];
            break;
        case ELightMode::Breathing:
        case ELightMode::ColorChange:
            _speed = (_currentLightEffect->isRandomSpeed
                      ? _breathingSpeeds[random(_breathingSpeeds.size())] : _currentLightEffect->speed) /
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

bool NeoPixelTask::isValidColorSet() {
    std::vector<ColorSet> *colorSets = &_currentLightEffect->colorSets;

    if (_currentLightEffect->isRandomColor) {
        if (_currentLightEffect->mode == ELightMode::Breathing) colorSets = &_defaultBreathingLightEffect.colorSets;
        if (_currentLightEffect->mode == ELightMode::Blinking) colorSets = &_defaultBlinkingLightEffect.colorSets;
    }

    return !colorSets->empty();
}
