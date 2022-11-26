//
// Created by jepanglee on 2022-11-26.
//

#ifndef MILKYWAY_AUDIOMSGQUEUE_H
#define MILKYWAY_AUDIOMSGQUEUE_H

#include "MsgQueue.h"
#include "AudioControl.h"

class AudioMsgData{
public:
    Playlist list;
    AudioMQEvents events;
};

class AudioMsgQueue : public MsgQueue<AudioMsgData> {
public:
    AudioMsgQueue(int length) : MsgQueue(length, sizeof(AudioMsgData*)) {
    }
};


#endif //MILKYWAY_AUDIOMSGQUEUE_H
