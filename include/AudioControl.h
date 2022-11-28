#ifndef MILKYWAY_AUDIO_H
#define MILKYWAY_AUDIO_H

#include <Audio.h>

class Sound {
public:
  String filename;
  int id;
};

class Playlist {
public:
  long id;
  bool isShuffle;
  std::vector<Sound> sounds;
};

class AudioControl{

public:
    AudioControl(int lrcPin,int blckPin,int doutPin);

    void setVolume(uint8_t volume);

    void setPlayList(Playlist& list);
    void pause(){
      if(_isResume){
        _isResume = false;
        _audio.pauseResume();
      }
    };
    void resume(){
      if(!_isResume){
        _isResume = true;
        _audio.pauseResume();
      }
    };
    
    void playNext();
    
    void loop();
    int16_t getLastGatin(){
      return _audio.getLastGain();
    }

private:
    Audio _audio;

    int _volume = 10;
    int _listIndex = 1;

    bool _isResume = true;

    Playlist _playList;
    
    const int _lrcPin;
    const int _blckPin;
    const int _doutPin;
    
    static const uint8_t maxVolume_ = 21;
    static const uint8_t minVolume_ = 0;
};

#endif //MILKYWAY_AUDIO_H