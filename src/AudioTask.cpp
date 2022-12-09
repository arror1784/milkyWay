#include "AudioTask.h"

AudioTask::AudioTask() : _audioControl(I2S_LRC, I2S_BCLK, I2S_DOUT), _msgQueue(30), _pingPongMsgQueue(30) {
    _audioControl.setVolume(_volume);
}

void AudioTask::sendMsg(AudioMsgData *dataA) {
    _msgQueue.send(dataA);
}

void AudioTask::task() {
    AudioMsgData *msg = _msgQueue.recv();

    if (_audioControl.isSDAccessing()) {
        if (msg != nullptr) delete msg;

        return;
    }
    if (msg != nullptr) {
        if (msg->events == EAudioMQEvent::UPDATE_VOLUME) {
            Serial.println("EAudioMQEvent::UPDATE_VOLUME");
            _audioControl.setVolume(msg->volume);
            _audioControl.setIsShuffle(msg->isShuffle);
        }
        else if (msg->events == EAudioMQEvent::UPDATE_PLAYLIST) {
            Serial.println("EAudioMQEvent::UPDATE_PLAYLIST");
            auto &list = msg->list;
            bool status = _audioControl.setPlayList(list);
            if (status && _isEnabled) {
                play();
            }
        }
        else if (msg->events == EAudioMQEvent::UPDATE_ENABLE) {
            Serial.println("EAudioMQEvent::UPDATE_ENABLE");
            _isEnabled = msg->enable;
            if (_isEnabled) {
                if (!_isCurrentFileDeleted) {
                    _audioControl.resume();
                }
                else {
                    play();
                }
                _isPingPong = msg->isPingPong;

                if (_isPingPong) {
                    _nextTick = millis() + _pingPongAudioTIme;
                }
            }
            else {
                _isEnabled = false;
                _audioControl.pause();
                _nextTick = 0xFFFFFFFF;
            }
        }
        else if (msg->events == EAudioMQEvent::UPDATE_DELETE_CURRENT_SOUND) {
            _audioControl.pause();
            _isCurrentFileDeleted = true;
            _nextTick = 0xFFFFFFFF;
        }

        delete msg;
    }
    _audioControl.loop();

    if (UserModeControl::getInstance().interactionMode == EInteractionMode::Synchronization
        && _audioControl.isPlaying()) {
        if (_syncUpdateResolution < pdTICKS_TO_MS(xTaskGetTickCount() - _tick)) {
            auto gain = std::abs(_audioControl.getLastGain()) / _volume;

            _tick = xTaskGetTickCount();
            auto *dataN = new NeoPixelMsgData();
            dataN->lightEffect = LightEffect();
            dataN->events = ENeoPixelMQEvent::UPDATE_SYNC;
            dataN->enable = true;

            if (_gains.size() > 10) {
                _gains.erase(_gains.begin());
            }
            _gains.push_back(gain);

            int total = 0;
            for (auto savedGain: _gains) {
                total += savedGain;
            }

            dataN->sync = total / _gains.size();

            NeoPixelTask::getInstance().sendSyncMsg(dataN);
        }
    }

    if (_isPingPong && _nextTick <= millis()) {
        auto *dataS = new PingPongMsgData();

        dataS->events = EPingPongMQEvent::FINISH_SOUND;
        dataS->enable = true;

        _pingPongMsgQueue.send(dataS);

        _isPingPong = false;
        setNextTick(0xFFFFFFFF);

        _audioControl.pause();
    }
}

PingPongMsgData *AudioTask::getPingPongMsg() {
    return _pingPongMsgQueue.recv();
}

void AudioTask::setIsSDAccessing(bool isSDAccessing) {
    _audioControl.setIsSDAccessing(isSDAccessing);
}

void AudioTask::setNextTick(unsigned long tick) {
    _nextTick = tick;
}

void AudioTask::play() {
    _audioControl.play();
}

const Sound &AudioTask::getCurrentSound() {
    return _audioControl.getCurrentSound();
}

void audio_info(const char *info) {
    Serial.print("info        ");
    Serial.println(info);
}

void audio_eof_mp3(const char *info) {  //end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
    AudioTask::getInstance().play();
}
