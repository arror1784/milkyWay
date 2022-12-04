#ifndef MILKYWAY_MSGQUEUES_H
#define MILKYWAY_MSGQUEUES_H

#include "AudioMsgQueue.h"
#include "ShuffleMsgQueue.h"
#include "NeoPixelMsgQueue.h"

class MsgQueues {
public:
    static void sendAudioMsg(AudioMsgData *data) { audioMsgQueue_.send(data); }

    static void sendShuffleMsg(ShuffleMsgData *data) { shuffleMsgQueue_.send(data); }

    static void sendNeoPixelMsg(NeoPixelMsgData *data) { neoPixelMsgQueue_.send(data); }

    static AudioMsgData *getAudioMsg() { audioMsgQueue_.recv(); }

    static ShuffleMsgData *getShuffleMsg() { shuffleMsgQueue_.recv(); }

    static NeoPixelMsgData *getNeoPixelMsg() { neoPixelMsgQueue_.recv(); }

private:
    static AudioMsgQueue audioMsgQueue_;
    static ShuffleMsgQueue shuffleMsgQueue_;
    static NeoPixelMsgQueue neoPixelMsgQueue_;
};

#endif //MILKYWAY_MSGQUEUES_H
