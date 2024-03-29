#ifndef MILKYWAY_AUDIOMSGQUEUE_H
#define MILKYWAY_AUDIOMSGQUEUE_H

#include "MsgQueue.h"
#include "AudioControl.h"

enum class EAudioMQEvent {
    UPDATE_ENABLE = 0,
    UPDATE_PLAYLIST = 1,
    UPDATE_CONFIG = 2,
    UPDATE_DELETE_CURRENT_SOUND = 3
};

class AudioMsgData {
public:
    Playlist list;
    EAudioMQEvent events;
    bool enable = true;
    bool isShuffle = false;
    int volume = -1;
};

class AudioMsgQueue : public MsgQueue<AudioMsgData> {
public:
    AudioMsgQueue(int length) : MsgQueue(length, sizeof(AudioMsgData *)) {}
};


#endif //MILKYWAY_AUDIOMSGQUEUE_H
