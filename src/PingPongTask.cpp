#include "PingPongTask.h"

void PingPongTask::task() {
    PingPongMsgData *msg = PingPongMsgQueue::getInstance().recv();

    if (msg != nullptr) {
        if (msg->events == EPingPongMQEvent::UPDATE_ENABLE) {
            _isEnabled = msg->enable;
            if (_isEnabled) {
                Serial.println("EPingPongMQEvent::UPDATE_ENABLE");
                Serial.println("_isEnabled : " + String(msg->enable));

                auto *dataA = new AudioMsgData();
                dataA->events = EAudioMQEvent::UPDATE_ENABLE;
                AudioTask::getInstance().sendMsg(dataA);
                _nextTick = millis() + _pingPongAudioTIme;
                _status = EPingPongStatus::SLEEP;
                _nextStatus = EPingPongStatus::NEO_PIXEL;
            }
            else {
                _nextTick = 0xFFFFFFFF;
            }
        }
        else if (msg->events == EPingPongMQEvent::FINISH_NEO_PIXEL) {
            Serial.println("EPingPongMQEvent::FINISH_NEO_PIXEL");
            if (_isEnabled) {
                _neoPixelCount -= 1;
                if (_neoPixelCount == 0) {
                    _status = EPingPongStatus::SLEEP;
                    _nextStatus = EPingPongStatus::AUDIO;
                    _nextTick = millis() + _pingPongSleepTIme;
                }
            }
        }
        delete msg;
    }

    if (_nextTick <= millis()) {
        AudioMsgData *dataA;
        NeoPixelMsgData *dataN;
        switch (_status) {
            case EPingPongStatus::SLEEP:
                _nextTick = _pingPongSleepTIme;
                break;
            case EPingPongStatus::AUDIO:
                _nextTick = millis() + _pingPongAudioTIme;
                dataA = new AudioMsgData();
                dataA->events = EAudioMQEvent::UPDATE_ENABLE;
                dataA->enable = true;
                AudioTask::getInstance().sendMsg(dataA);
                break;
            case EPingPongStatus::NEO_PIXEL:
                _neoPixelCount = _neoPixelCountCycle;
                dataN = new NeoPixelMsgData();
                dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;
                dataN->enable = true;
                NeoPixelTask::getInstance().sendMsg(dataN);
                break;
        }
        _status = _nextStatus;
        _nextStatus = EPingPongStatus::SLEEP;
    }
}
