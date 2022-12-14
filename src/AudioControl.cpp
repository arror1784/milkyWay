#include "AudioControl.h"


AudioControl::AudioControl(int lrcPin, int blckPin, int doutPin)
    : _lrcPin(lrcPin), _blckPin(blckPin), _doutPin(doutPin) {
    _audio.setPinout(blckPin, lrcPin, doutPin);
//    _audio.forceMono(true);
}

void AudioControl::setVolume(uint8_t volume) {
    _audio.setVolume(volume);
}

bool AudioControl::setPlayList(Playlist &list) {
    bool isSamePlaylist = true;

    if (list.sounds.size() == _playList.sounds.size()) {
        for (int i = 0; i < list.sounds.size(); i++) {
            if (_playList.sounds.size() - 1 < i) {
                isSamePlaylist = false;
                break;
            }
            if (_playList.sounds[i].id != list.sounds[i].id) {
                isSamePlaylist = false;
                break;
            }
        }
    }
    else {
        isSamePlaylist = false;
    }

    if (isSamePlaylist) return false;

    _playList = list;

    return true;
}

void AudioControl::pause() {
    if (!isValidPlaylist()) return;
    if (_isResume) {
        Serial.println("AudioControl::pause : pause");
        _isResume = false;
        _audio.pauseResume();
    }
}

void AudioControl::resume() {
    if (!isValidPlaylist()) return;
    if (!_isResume) {
        Serial.println("AudioControl::resume : resume");
        _isResume = true;
        _audio.pauseResume();
    }
};

void AudioControl::updatePlaylistIndex() {
    Serial.println("AudioControl::updatePlaylistIndex : updatePlaylistIndex");
    if (_isShuffle) {
        _listIndex = random((long) _playList.sounds.size());
    }
    else {
        _listIndex++;
        if (_listIndex >= _playList.sounds.size()) _listIndex = 0;
    }
}

void AudioControl::play() {
    _audio.stopSong();

    if (!isValidPlaylist()) return;

    Serial.println("AudioControl::play : play");
    _isResume = true;
    updatePlaylistIndex();
    _audio.connecttoFS(SD, String("/" + _playList.sounds[_listIndex].filename).c_str());
}

void AudioControl::loop() {
    _audio.loop();
}

int16_t AudioControl::getLastGain() {
    return _audio.getLastGain();
}

const Sound &AudioControl::getCurrentSound() {
    return _playList.sounds[_listIndex];
}

bool AudioControl::isPlaying() {
    return _isResume;
}

bool AudioControl::isSDAccessing() {
    return _isSDAccessing;
}

void AudioControl::setIsSDAccessing(bool isSDAccessing) {
    _isSDAccessing = isSDAccessing;
}

bool AudioControl::isValidPlaylist() {
    return !_playList.sounds.empty();
}

void AudioControl::setIsShuffle(bool isShuffle) {
    _isShuffle = isShuffle;
}
