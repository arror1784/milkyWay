#include "PingPongTask.h"

void PingPongTask::task() {
    PingPongMsgData *msg = PingPongMsgQueue::getInstance().recv();

    if (msg != nullptr) {
        if (msg->events == EPingPongMQEvent::UPDATE_ENABLE) {
            _isEnabled = msg->enable;
            if (_isEnabled) {
                Serial.println("PingPongTask::task : EPingPongMQEvent::UPDATE_ENABLE" + String(msg->enable));

                auto *dataA = new AudioMsgData();
                dataA->events = EAudioMQEvent::UPDATE_ENABLE;
                AudioTask::getInstance().sendMsg(dataA);

                _status = EPingPongStatus::AUDIO;
                _lastStatus = EPingPongStatus::AUDIO;
                _nextTick = 0;
            }
            else {
                _nextTick = 0xFFFFFFFF;
            }
        }
        else if (msg->events == EPingPongMQEvent::FINISH_NEO_PIXEL) {
            Serial.println("PingPongTask::task : EPingPongMQEvent::FINISH_NEO_PIXEL");
            if (_isEnabled) {
                _neoPixelCount -= 1;
                if (_neoPixelCount == 0) {

                    _status = EPingPongStatus::SLEEP;
                    _nextTick = 0;
                    _neoPixelCount = -1;
                }
            }
        }
        delete msg;
    }

    if (_nextTick <= millis()) {
        auto *dataA = new AudioMsgData();
        dataA->events = EAudioMQEvent::UPDATE_ENABLE;

        auto *dataN = new NeoPixelMsgData();
        dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;

        switch (_status) {
            case EPingPongStatus::SLEEP:
                _nextTick = millis() + _pingPongSleepTIme;

                if(_lastStatus == EPingPongStatus::AUDIO){
                    _status = EPingPongStatus::NEO_PIXEL;
                    _lastStatus = EPingPongStatus::NEO_PIXEL;
                }
                else{
                    _status = EPingPongStatus::AUDIO;
                    _lastStatus = EPingPongStatus::AUDIO;
                }

                dataA->enable = false;
                dataN->enable = false;

                break;
            case EPingPongStatus::AUDIO:
                _nextTick = millis() + _pingPongAudioTIme;

                dataA->enable = true;
                dataN->enable = false;

                _status = EPingPongStatus::SLEEP;

                break;

            case EPingPongStatus::NEO_PIXEL:
                _neoPixelCount = _neoPixelCountCycle;
                _nextTick = 0xFFFFFFFF;

                dataA->enable = false;
                dataN->enable = true;
                dataN->count = _neoPixelCount;

                _status = EPingPongStatus::SLEEP;

                break;
        }
        AudioTask::getInstance().sendMsg(dataA);
        NeoPixelTask::getInstance().sendMsg(dataN);
    }
}
