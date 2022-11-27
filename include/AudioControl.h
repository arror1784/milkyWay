#ifndef MILKYWAY_AUDIO_H
#define MILKYWAY_AUDIO_H

#include <Audio.h>

class Sound {
public:
  String filename;
  long size;
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

    void loop();

private:
    Audio _audio;

    int _volume = 10;
    int _listIndex = 0;

    Playlist _playList;
    
    const int _lrcPin;
    const int _blckPin;
    const int _doutPin;
    
    static const uint8_t maxVolume_ = 21;
    static const uint8_t minVolume_ = 0;
};

#endif //MILKYWAY_AUDIO_H