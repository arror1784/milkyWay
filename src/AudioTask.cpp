#include "AudioTask.h"

AudioTask::AudioTask() : _audioControl(I2S_LRC, I2S_BCLK, I2S_DOUT), _msgQueue(30), _shuffleMsgQueue(30) {
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
            _audioControl.setVolume(msg->volume);
            _audioControl.setIsShuffle(msg->isShuffle);
        }
        else if (msg->events == EAudioMQEvent::UPDATE_PLAYLIST) {
            auto &list = msg->list;
            bool status = _audioControl.setPlayList(list);
            if (status) {
                play();
            }
        }
        else if (msg->events == EAudioMQEvent::UPDATE_ENABLE) {
            if (msg->enable) {
                if(!_isCurrentFileDeleted) {
                    _audioControl.resume();
                } else {
                    play();
                }
                _isShuffle = msg->isShuffle;

                if (_isShuffle) {
                    _nextTick = millis() + _shuffleAudioTIme;
                }
            }
            else {
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

    if (_isShuffle && _nextTick <= millis()) {
        auto *dataS = new ShuffleMsgData();

        dataS->events = EShuffleSMQEvent::FINISH_SOUND;
        dataS->enable = true;

        _shuffleMsgQueue.send(dataS);

        _isShuffle = false;
        setNextTick(0xFFFFFFFF);

        _audioControl.pause();
    }
}

ShuffleMsgData *AudioTask::getShuffleMsg() {
    return _shuffleMsgQueue.recv();
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
