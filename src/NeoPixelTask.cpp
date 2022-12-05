#include "NeoPixelTask.h"

NeoPixelTask::NeoPixelTask() : _neoPixel(LED_LENGTH, LED_PIN, NEO_GRBW | NEO_KHZ800), _shuffleMsgQueue(5), _msgQueue(5) {
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

void NeoPixelTask::sendMsg(NeoPixelMsgData *dataN) {
    _msgQueue.send(dataN);
}

void NeoPixelTask::reset() {
    _neoPixel.off();
    setNextTick(0xFFFFFFFF);
}

const LightEffect &NeoPixelTask::getLightEffect(ELightMode mode) {
    switch (mode) {
        case ELightMode::Breathing:
            return _breathingLightEffect.id == 0
                   ? _defaultBreathingLightEffect : _breathingLightEffect;
        case ELightMode::Blinking:
            return _blinkingLightEffect.id == 0
                   ? _defaultBlinkingLightEffect : _blinkingLightEffect;
        case ELightMode::ColorChange:
            return _colorChangeLightEffect.id == 0
                   ? _defaultColorChangeLightEffect : _colorChangeLightEffect;
        default:
            break;
    }

    auto randomValue = random(3);

    if (randomValue == 0) return getLightEffect(ELightMode::Breathing);
    if (randomValue == 1) return getLightEffect(ELightMode::Blinking);
    return getLightEffect(ELightMode::ColorChange);
}

unsigned int NeoPixelTask::getSpeed() {
    const LightEffect &lightEffect = getLightEffect(_mode);

    switch (lightEffect.mode) {
        case ELightMode::Blinking:
            return lightEffect.isRandomSpeed
                   ? _blinkingSpeeds[random(_blinkingSpeeds.size())] : lightEffect.speed;
        case ELightMode::Breathing:
        case ELightMode::ColorChange:
            return lightEffect.isRandomSpeed
                   ? _breathingSpeeds[random(_breathingSpeeds.size())] : lightEffect.speed;
        default:
            break;
    }
    return lightEffect.speed;
}

void NeoPixelTask::setLightEffect(const LightEffect &lightEffect) {
    ELightMode mode = lightEffect.mode;

    if (mode == ELightMode::Breathing) _breathingLightEffect = lightEffect;
    if (mode == ELightMode::Blinking) _blinkingLightEffect = lightEffect;
    if (mode == ELightMode::ColorChange) _colorChangeLightEffect = lightEffect;

    refreshColorSet();
}

void NeoPixelTask::refreshColorSet() {
    const LightEffect &lightEffect = getLightEffect(_mode);

    auto randomColorSetIndex = random((long) lightEffect.colorSets.size());
    _neoPixel.setColorSet(lightEffect.colorSets[randomColorSetIndex]);
}

void NeoPixelTask::refreshMode() {
    refreshColorSet();

    unsigned int speed = getSpeed();

    if (_mode == ELightMode::Blinking) {
        _previousSpeed = speed;
    }
    else if (_mode == ELightMode::Breathing || _mode == ELightMode::ColorChange) {
        _previousSpeed = speed / _neoPixel.getMaxBrightness();
    }
    addNextTick(_previousSpeed);
}

void NeoPixelTask::task() {
    NeoPixelMsgData *msg = _msgQueue.recv();

    if (msg != nullptr) {
        if (msg->events == ENeoPixelMQEvent::UPDATE_EFFECT) {
            setLightEffect(msg->lightEffect);
        }
        else if (msg->events == ENeoPixelMQEvent::UPDATE_MODE) {
            _mode = msg->mode;
            refreshMode();
        }
        else if (msg->events == ENeoPixelMQEvent::UPDATE_ENABLE) {
            if (msg->enable) {
                _isShuffle = msg->isShuffle;
                if (_isShuffle) {
                    _count = _oneCycleCount;
                }
                refreshMode();
            }
            else {
                reset();
            }
        }
        else if (msg->events == ENeoPixelMQEvent::UPDATE_SYNC) {
//            _neoPixel.on(msg->sync);
        }
        delete msg;
    }

    if (millis() >= _nextTick) {
        ticked();
    }
}

void NeoPixelTask::ticked() {
    if (_mode == ELightMode::Blinking) {
        if (_neoPixel.isOn()) {
            _neoPixel.off();
            _previousSpeed = getSpeed();
        }
        else {
            _neoPixel.on();
            if (_count > 0) {
                _count -= 1;
            }
            else if (_count == 0) {
                finishCycle();
                setNextTick(0xFFFFFFFF);
                return;
            }
        }
    }
    else if (_mode == ELightMode::Breathing || _mode == ELightMode::ColorChange) {
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
            if (_mode == ELightMode::ColorChange) {
                refreshColorSet();
            }

            if (_count > 0) {
                _count -= 1;
            }
            else if (_count == 0) {
                finishCycle();
                setNextTick(0xFFFFFFFF);
                return;
            }
            _previousSpeed = getSpeed() / maxBrightness;
        }
    }
    addNextTick(_previousSpeed);
}

void NeoPixelTask::addNextTick(unsigned long speed) {
    _nextTick = millis() + speed;
}

void NeoPixelTask::setNextTick(unsigned long tick) {
    _nextTick = tick;
}

void NeoPixelTask::finishCycle() {
    if (_isShuffle) {
        auto dataS = new ShuffleMsgData();

        dataS->enable = true;
        dataS->events = EShuffleSMQEvent::FINISH_NEO_PIXEL;

        _shuffleMsgQueue.send(dataS);

        _isShuffle = false;

        reset();
    }
}

ShuffleMsgData *NeoPixelTask::getShuffleMsg() {
    return _shuffleMsgQueue.recv();
}
