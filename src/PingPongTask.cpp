#include "PingPongTask.h"

#include "EepromControl.h"
#include "ToServerMsgQueue.h"

// pingPongLastModePath_ 0이면 네오픽셀
// pingPongLastModePath_ 1이면 오디오

void PingPongTask::task() {
    PingPongMsgData *msg = PingPongMsgQueue::getInstance().recv();

    if (msg != nullptr) {
        if (msg->events == EPingPongMQEvent::UPDATE_ENABLE) {
            _isEnabled = msg->enable;
            if (_isEnabled) {
                SERIAL_PRINTLN("PingPongTask::task : EPingPongMQEvent::UPDATE_ENABLE" + String(msg->enable));

                auto *dataA = new AudioMsgData();
                dataA->events = EAudioMQEvent::UPDATE_ENABLE;
                AudioTask::getInstance().sendMsg(dataA);

                // EPingPongStatus::NEO_PIXEL = 0
                if (EepromControl::getInstance().getPingPongStartMode() == 0) {
                    _status = EPingPongStatus::AUDIO;
                    _statusAudioOrLED = EPingPongStatus::AUDIO;
                }
                else {
                    _status = EPingPongStatus::NEO_PIXEL;
                    _statusAudioOrLED = EPingPongStatus::NEO_PIXEL;
                }

                _nextTick = 0;
            }
            else {
                _nextTick = 0xFFFFFFFF;
            }
        }
        else if (msg->events == EPingPongMQEvent::FINISH_NEO_PIXEL) {
            SERIAL_PRINTLN("PingPongTask::task : EPingPongMQEvent::FINISH_NEO_PIXEL");
            if (_isEnabled) {
                _neoPixelCount -= 1;
                if (_neoPixelCount == 0) {

                    _status = EPingPongStatus::SLEEP;
                    _nextTick = 0;
                    _neoPixelCount = -1;
                }
            }
            SERIAL_PRINTLN("PingPongTask::task : send to server");
            auto *data = new ToServerMsgData();
            ToServerMsgQueue::getInstance().send(data);
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

                if (_statusAudioOrLED == EPingPongStatus::AUDIO) {
                    _status = EPingPongStatus::NEO_PIXEL;
                    _statusAudioOrLED = EPingPongStatus::NEO_PIXEL;
                    // EPingPongStatus::NEO_PIXEL = 0
                    EepromControl::getInstance().setPingPongStartMode(0);

                }
                else {
                    _status = EPingPongStatus::AUDIO;
                    _statusAudioOrLED = EPingPongStatus::AUDIO;
                    // EPingPongStatus::AUDIO = 1
                    EepromControl::getInstance().setPingPongStartMode(1);


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
