#ifndef MILKYWAY_AUDIO_DOWNLOAD_MSG_QUEUE_H
#define MILKYWAY_AUDIO_DOWNLOAD_MSG_QUEUE_H

#include "MsgQueue.h"

class AudioDownloadMsgData {
public:
    long id;
    String filename;
};

class AudioDownloadMsgQueue : public MsgQueue<AudioDownloadMsgData> {
public:
    AudioDownloadMsgQueue(int length) : MsgQueue(length, sizeof(AudioDownloadMsgData *)) {}
};


#endif //MILKYWAY_AUDIO_DOWNLOAD_MSG_QUEUE_H
