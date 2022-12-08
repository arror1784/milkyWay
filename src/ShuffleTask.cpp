#include "ShuffleTask.h"

ShuffleTask::ShuffleTask() : _msgQueue(30) {

}

void ShuffleTask::sendMsg(ShuffleMsgData *dataN) {
    _msgQueue.send(dataN);
}

void ShuffleTask::task() {
    ShuffleMsgData *dataS = _msgQueue.recv();

    if (dataS != nullptr) {
        if (dataS->events == EShuffleSMQEvent::UPDATE_ENABLE) {
            if (dataS->enable) {
                Serial.println("EShuffleSMQEvent::UPDATE_ENABLE");
                auto *dataA = new AudioMsgData();
                dataA->isShuffle = true;
                AudioTask::getInstance().sendMsg(dataA);

                auto *dataN = new NeoPixelMsgData();
                dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;
                dataN->enable = false;
                dataN->isShuffle = true;
                NeoPixelTask::getInstance().sendMsg(dataN);
            }
        }

        delete dataS;
    }

    dataS = NeoPixelTask::getInstance().getShuffleMsg();

    if (dataS != nullptr) {
        if (dataS->events == EShuffleSMQEvent::FINISH_NEO_PIXEL) {
            Serial.println("EShuffleSMQEvent::FINISH_NEO_PIXEL");
            _isNextSound = true;
            _nextTick = millis() + _shuffleSleepTIme;
        }
        delete dataS;
    }

    dataS = AudioTask::getInstance().getShuffleMsg();

    if (dataS != nullptr) {
        if (dataS->events == EShuffleSMQEvent::FINISH_SOUND) {
            Serial.println("EShuffleSMQEvent::FINISH_SOUND");
            _isNextSound = false;
            _nextTick = millis() + _shuffleSleepTIme;
        }

        delete dataS;
    }

    if (_nextTick <= millis()) {
        Serial.println("_isNextSound : " + String(_isNextSound));
        if (_isNextSound) {
            auto *dataA = new AudioMsgData();
            dataA->events = EAudioMQEvent::UPDATE_ENABLE;
            dataA->enable = true;
            dataA->isShuffle = true;
            AudioTask::getInstance().sendMsg(dataA);
        }
        else {
            auto *dataN = new NeoPixelMsgData();
            dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;
            dataN->enable = true;
            dataN->isShuffle = true;
            NeoPixelTask::getInstance().sendMsg(dataN);
        }
        _nextTick = 0xFFFFFFFF;
    }
}
