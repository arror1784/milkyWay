#include "AudioControl.h"


AudioControl::AudioControl(int lrcPin,int blckPin,int doutPin) : _lrcPin(lrcPin), _blckPin(blckPin), _doutPin(doutPin){

    _audio.setPinout(lrcPin,blckPin,doutPin);

}

void AudioControl::setVolume(uint8_t volume){
    _volume = volume;
    _audio.setVolume(volume);

}

void AudioControl::loop(){
    // if(!_enable)
        // return;
    _audio.loop();
}

void AudioControl::setPlayList(Playlist& list){
    _playList = list;
    _listIndex = 0;
}

void AudioControl::playNext() {
    Serial.println("Asdasdasd");
    if(_playList.sounds.size() == 0)
        return;
    _audio.connecttoFS(SD,String("/" + String(_playList.sounds[_listIndex].id) + "_" +_playList.sounds[_listIndex].filename).c_str());
    _listIndex++;
    if(_listIndex >= _playList.sounds.size())
        _listIndex = 0;
    Serial.println("Asdasdasd");
}
