#include "AudioTask.h"

AudioTask::AudioTask(): _msgQueue(5), _audioControl(I2S_LRC, I2S_BCLK, I2S_DOUT) {
    _audioControl.setVolume(volume);
}

void AudioTask::sendMsg(AudioMsgData *dataA) {
    _msgQueue.send(dataA);
}

void AudioTask::task() {
    AudioMsgData *msg = _msgQueue.recv();

    if (_audioControl.isDownloading()) {
        if (msg != nullptr) delete msg;

        return;
    }
    if (msg != nullptr) {
        if (msg->events == EAudioMQEvent::UPDATE_PLAYLIST) {
            Serial.println("updatePLAYLIST");
            auto &list = msg->list;
            _audioControl.setPlayList(list);
        }
        else if (msg->events == EAudioMQEvent::UPDATE_ENABLE) {
            if (msg->enable) {
                _audioControl.resume();
                isShuffle = msg->isShuffle;

                if (isShuffle) {
                    nextTick = millis() + _shuffleAudioTIme;
                }
            }
            else {
                _audioControl.pause();
            }
        }

        delete msg;
    }
    _audioControl.loop();

    if (UserModeControl::getInstance().interactionMode == EInteractionMode::Synchronization) {
        if (_syncUpdateResolution < pdTICKS_TO_MS(xTaskGetTickCount() - tick)) {
            auto gain = std::abs(_audioControl.getLastGain()) / volume;

            tick = xTaskGetTickCount();
            auto *dataN = new NeoPixelMsgData();
            dataN->lightEffect = LightEffect();
            dataN->events = ENeoPixelMQEvent::UPDATE_SYNC;
            dataN->mode = ELightMode::None;

            if (gains.size() > 10) {
                gains.erase(gains.begin());
            }
            gains.push_back(gain);

            int total = 0;
            for (auto savedGain: gains) {
                total += savedGain;
            }

            dataN->sync = total / gains.size();

            NeoPixelTask::getInstance().sendMsg(dataN);
        }
    }
//    if (isShuffle && nextTick <= millis()) {
//        auto *dataS = new ShuffleMsgData();
//
//        dataS->events = EShuffleSMQEvent::FINISH_SOUND;
//        dataS->enable = true;
//
//        shuffleMsgQueue.send(dataS);
//
//        nextTick = 0xFFFFFFFF;
//    }
}

void AudioTask::setIsDownloading(bool isDownloading) {
    _audioControl.setIsDownloading(isDownloading);
}
