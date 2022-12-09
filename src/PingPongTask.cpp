#include "PingPongTask.h"

PingPongTask::PingPongTask() : _msgQueue(30) {

}

void PingPongTask::sendMsg(PingPongMsgData *dataN) {
    _msgQueue.send(dataN);
}

void PingPongTask::task() {
    PingPongMsgData *dataS = _msgQueue.recv();

    if (dataS != nullptr) {
        if (dataS->events == EPingPongMQEvent::UPDATE_ENABLE) {
            if (dataS->enable) {
                Serial.println("EPingPongMQEvent::UPDATE_ENABLE");
                auto *dataA = new AudioMsgData();
                dataA->events = EAudioMQEvent::UPDATE_ENABLE;
                dataA->isPingPong = true;
                AudioTask::getInstance().sendMsg(dataA);

                auto *dataN = new NeoPixelMsgData();
                dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;
                dataN->enable = false;
                dataN->isPingPong = true;
                NeoPixelTask::getInstance().sendMsg(dataN);
            }
        }

        delete dataS;
    }

    dataS = NeoPixelTask::getInstance().getPingPongMsg();

    if (dataS != nullptr) {
        if (dataS->events == EPingPongMQEvent::FINISH_NEO_PIXEL) {
            Serial.println("EPingPongMQEvent::FINISH_NEO_PIXEL");
            _isNextSound = true;
            _nextTick = millis() + _pingPongSleepTIme;
        }
        delete dataS;
    }

    dataS = AudioTask::getInstance().getPingPongMsg();

    if (dataS != nullptr) {
        if (dataS->events == EPingPongMQEvent::FINISH_SOUND) {
            Serial.println("EPingPongMQEvent::FINISH_SOUND");
            _isNextSound = false;
            _nextTick = millis() + _pingPongSleepTIme;
        }

        delete dataS;
    }

    if (_nextTick <= millis()) {
        Serial.println("_isNextSound : " + String(_isNextSound));
        if (_isNextSound) {
            auto *dataA = new AudioMsgData();
            dataA->events = EAudioMQEvent::UPDATE_ENABLE;
            dataA->enable = true;
            dataA->isPingPong = true;
            AudioTask::getInstance().sendMsg(dataA);
        }
        else {
            auto *dataN = new NeoPixelMsgData();
            dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;
            dataN->enable = true;
            dataN->isPingPong = true;
            NeoPixelTask::getInstance().sendMsg(dataN);
        }
        _nextTick = 0xFFFFFFFF;
    }
}
