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

class AudioControl {
public:
    AudioControl(int lrcPin, int blckPin, int doutPin);

    void setVolume(uint8_t volume);

    bool setPlayList(Playlist &list);

    void pause();

    void resume();

    void updatePlaylistIndex();

    void play();

    void loop();

    int16_t getLastGain();

    bool isPlaying();

    bool isDownloading();

    void setIsSDAccessing(bool isDownloading);

private:

    bool isValidPlaylist();

    Audio _audio;

    int _listIndex = 1;

    bool _isResume = true;
    bool _isSDAccessing = false;

    Playlist _playList;

    const int _lrcPin;
    const int _blckPin;
    const int _doutPin;

    static const uint8_t maxVolume_ = 21;
    static const uint8_t minVolume_ = 0;
};

#endif //MILKYWAY_AUDIO_H