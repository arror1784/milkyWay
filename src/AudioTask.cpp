#include "AudioTask.h"

AudioTask::AudioTask() : _audioControl(I2S_LRC, I2S_BCLK, I2S_DOUT), _msgQueue(30) {
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
            handlePlayStatus(true);
        }
        else if (msg->events == EAudioMQEvent::UPDATE_PLAYLIST) {
            Serial.println("EAudioMQEvent::UPDATE_PLAYLIST");
            bool status = _audioControl.setPlayList(msg->list);
            if (status) {
                _shouldChangeSound = true;
            }
            handlePlayStatus(status);
        }
        else if (msg->events == EAudioMQEvent::UPDATE_ENABLE) {
            Serial.println("EAudioMQEvent::UPDATE_ENABLE");
            Serial.println("_isEnabled : " + String(msg->enable));
            _isEnabled = msg->enable;
            handlePlayStatus(true);
        }
        else if (msg->events == EAudioMQEvent::UPDATE_DELETE_CURRENT_SOUND) {
            Serial.println("EAudioMQEvent::UPDATE_DELETE_CURRENT_SOUND");
            _audioControl.pause();
            _shouldChangeSound = true;
        }

        delete msg;
    }
    _audioControl.loop();

    if (UserModeControl::getInstance().interactionMode == EInteractionMode::Synchronization && _isEnabled) {
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
}

void AudioTask::setIsSDAccessing(bool isSDAccessing) {
    _audioControl.setIsSDAccessing(isSDAccessing);
}

void AudioTask::handlePlayStatus(bool status) {
    Serial.println("handlePlayStatus");
    if (_isEnabled && status) {
        if (!_shouldChangeSound) {
            Serial.println("resume run");
            _audioControl.resume();
        }
        else {
            Serial.println("play run");
            _shouldChangeSound = false;
            _audioControl.play();
        }
    }
    else {
        Serial.println("pause run");
        _audioControl.pause();
    }
}

const Sound &AudioTask::getCurrentSound() {
    return _audioControl.getCurrentSound();
}

void AudioTask::setShouldChangeSound(bool shouldChangeSound) {
    _shouldChangeSound = shouldChangeSound;
}

void audio_info(const char *info) {
    Serial.print("info        ");
    Serial.println(info);
}

void audio_eof_mp3(const char *info) {  //end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
    AudioTask::getInstance().setShouldChangeSound(true);
    AudioTask::getInstance().handlePlayStatus(true);
}
