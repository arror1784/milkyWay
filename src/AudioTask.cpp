#include "AudioTask.h"
#include "ToServerMsgQueue.h"

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
        if (msg->events == EAudioMQEvent::UPDATE_CONFIG) {
            SERIAL_PRINTLN("AudioTask::task : EAudioMQEvent::UPDATE_CONFIG");
            this->setConfig(msg->volume, msg->isShuffle);
        }
        else if (msg->events == EAudioMQEvent::UPDATE_PLAYLIST) {
            bool status = _audioControl.setPlayList(msg->list);
            SERIAL_PRINTLN("AudioTask::task : EAudioMQEvent::UPDATE_PLAYLIST : " + String(status));
            if (status) {
                _shouldChangeSound = true;
                handlePlayStatus(status);
            }
        }
        else if (msg->events == EAudioMQEvent::UPDATE_ENABLE) {
            SERIAL_PRINTLN("AudioTask::task : EAudioMQEvent::UPDATE_ENABLE" + String(msg->enable));
            _isEnabled = msg->enable;
            handlePlayStatus(true);
            _cycleTick = _isEnabled ? xTaskGetTickCount() : 0;
        }
        else if (msg->events == EAudioMQEvent::UPDATE_DELETE_CURRENT_SOUND) {
            SERIAL_PRINTLN("AudioTask::task : EAudioMQEvent::UPDATE_DELETE_CURRENT_SOUND");
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

    if (_isEnabled && pdTICKS_TO_MS(xTaskGetTickCount() - _cycleTick) > 7000) {
        SERIAL_PRINTLN("AudioTask::task : send to server");
        _cycleTick = xTaskGetTickCount();
        auto *data = new ToServerMsgData();
        ToServerMsgQueue::getInstance().send(data);
    }
}

void AudioTask::setIsSDAccessing(bool isSDAccessing) {
    _audioControl.setIsSDAccessing(isSDAccessing);
}

void AudioTask::handlePlayStatus(bool status) {
    if (_isEnabled && status) {
        if (!_shouldChangeSound) {
            SERIAL_PRINTLN("AudioTask::handlePlayStatus : resume run");
            _audioControl.resume();
        }
        else {
            SERIAL_PRINTLN("AudioTask::handlePlayStatus : play run");
            _shouldChangeSound = false;
            _audioControl.play();
        }
    }
    else {
        SERIAL_PRINTLN("AudioTask::handlePlayStatus : pause run");
        _audioControl.pause();
    }
}

void AudioTask::setConfig(int volume, bool isShuffle) {
    _audioControl.setVolume(volume);
    _audioControl.setIsShuffle(isShuffle);
}

const Sound &AudioTask::getCurrentSound() {
    return _audioControl.getCurrentSound();
}

void AudioTask::setShouldChangeSound(bool shouldChangeSound) {
    _shouldChangeSound = shouldChangeSound;
}

void audio_info(const char *info) {
    SERIAL_PRINT("info        ");
    SERIAL_PRINTLN(info);
}

void audio_eof_mp3(const char *info) {  //end of file
    SERIAL_PRINT("eof_mp3     ");
    SERIAL_PRINTLN(info);
    AudioTask::getInstance().setShouldChangeSound(true);
    AudioTask::getInstance().handlePlayStatus(true);
}
