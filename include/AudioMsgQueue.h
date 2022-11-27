//
// Created by jepanglee on 2022-11-26.
//

#ifndef MILKYWAY_AUDIOMSGQUEUE_H
#define MILKYWAY_AUDIOMSGQUEUE_H

#include "MsgQueue.h"
#include "AudioControl.h"

enum class AudioMQEvents{
  UPDATE_ENABLE = 0,
  UPDATE_PLAYLIST = 1
};

class AudioMsgData{
public:
    Playlist list;
    AudioMQEvents events;
    bool enable = true;
};

class AudioMsgQueue : public MsgQueue<AudioMsgData> {
public:
    AudioMsgQueue(int length) : MsgQueue(length, sizeof(AudioMsgData*)) {
    }
};


#endif //MILKYWAY_AUDIOMSGQUEUE_H
