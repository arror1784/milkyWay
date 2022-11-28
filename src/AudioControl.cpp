#include "AudioControl.h"


AudioControl::AudioControl(int lrcPin,int blckPin,int doutPin) : _lrcPin(lrcPin), _blckPin(blckPin), _doutPin(doutPin){

    _audio.setPinout(blckPin,lrcPin,doutPin);
    _audio.forceMono(true);
}

void AudioControl::setVolume(uint8_t volume){
    _volume = volume;
    _audio.setVolume(volume);

}

void AudioControl::loop(){
    _audio.loop();
}

void AudioControl::setPlayList(Playlist& list){
    _playList = list;
    _listIndex = 0;
    // _audio.stopSong();

    // if(_isResume){
    //     playNext();
    // }else{
    //     playNext();
    // }
}

void AudioControl::playNext() {
    if(_playList.sounds.size() == 0)
        return;
    _audio.connecttoFS(SD,String("/" + String(_playList.sounds[_listIndex].id) + "_" +_playList.sounds[_listIndex].filename).c_str());
    _listIndex++;
    if(_listIndex >= _playList.sounds.size())
        _listIndex = 0;
}
