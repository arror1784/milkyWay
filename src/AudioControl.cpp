#include "AudioControl.h"


AudioControl::AudioControl(int lrcPin, int blckPin, int doutPin)
    : _lrcPin(lrcPin), _blckPin(blckPin), _doutPin(doutPin) {
    _audio.setPinout(blckPin, lrcPin, doutPin);
//    _audio.forceMono(true);
}

void AudioControl::setVolume(uint8_t volume) {
    _volume = volume;
    _audio.setVolume(volume);

}

void AudioControl::setPlayList(Playlist &list) {
    _playList = list;
    _listIndex = 0;

    _audio.stopSong();
    playNext();

    if (!_isResume) {
        _audio.pauseResume();
    }
}

void AudioControl::pause() {
    if (_isResume) {
        _isResume = false;
        _audio.pauseResume();
    }
}

void AudioControl::resume() {
    if (!_isResume) {
        _isResume = true;
        _audio.pauseResume();
    }
};

void AudioControl::playNext() {
    if (_playList.sounds.empty())
        return;
    _audio.connecttoFS(SD, String("/" + _playList.sounds[_listIndex].filename).c_str());
    _listIndex++;
    if (_listIndex >= _playList.sounds.size())
        _listIndex = 0;
}

void AudioControl::loop() {
    _audio.loop();
}

int16_t AudioControl::getLastGain() {
    return _audio.getLastGain();
}

bool AudioControl::isPlaying() {
    return _isResume;
}

bool AudioControl::isDownloading() {
    return _isDownloading;
}

void AudioControl::setIsDownloading(bool isDownloading) {
    _isDownloading = isDownloading;
}
