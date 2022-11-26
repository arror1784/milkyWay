#include "AudioControl.h"


AudioControl::AudioControl(int lrcPin,int blckPin,int doutPin) : _lrcPin(lrcPin), _blckPin(blckPin), _doutPin(doutPin){

    _audio.setPinout(lrcPin,blckPin,doutPin);

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
}