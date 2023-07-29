#include "AudioControl.h"


AudioControl::AudioControl(int lrcPin, int blckPin, int doutPin)
    : _lrcPin(lrcPin), _blckPin(blckPin), _doutPin(doutPin) {
    _audio.setPinout(blckPin, lrcPin, doutPin);
//    _audio.forceMono(true);
}

bool AudioControl::setVolume(uint8_t volume) {
    auto previousVolume = _audio.getVolume();
    _audio.setVolume(volume);
    return previousVolume != volume;
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
    _listIndex = (int) list.sounds.size() - 1;

    return true;
}

void AudioControl::pause() {
    if (!isValidPlaylist()) return;
    if (_isResume) {
        SERIAL_PRINTLN("AudioControl::pause : pause");
        _isResume = false;
        _audio.pauseResume();
    }
}

void AudioControl::resume() {
    if (!isValidPlaylist()) return;
    if (!_isResume) {
        SERIAL_PRINTLN("AudioControl::resume : resume");
        _isResume = true;
        _audio.pauseResume();
    }
};

void AudioControl::updatePlaylistIndex() {
    SERIAL_PRINTLN("AudioControl::updatePlaylistIndex : updatePlaylistIndex");
    SERIAL_PRINTLN("AudioControl::updatePlaylistIndex : _isShuffle : " + String(_isShuffle));
    if (_isShuffle) {
        _listIndex = random((long) _playList.sounds.size());
    }
    else {
        _listIndex++;
        if (_listIndex >= _playList.sounds.size()) _listIndex = 0;
    }
    SERIAL_PRINTLN("AudioControl::updatePlaylistIndex : _listIndex : " + String(_listIndex));
}

void AudioControl::play() {
    _audio.stopSong();

    if (!isValidPlaylist()) return;

    SERIAL_PRINTLN("AudioControl::play : play");
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

bool AudioControl::setIsShuffle(bool isShuffle) {
    auto previousIsShuffle = _isShuffle;
    _isShuffle = isShuffle;
    return previousIsShuffle != isShuffle;
}
