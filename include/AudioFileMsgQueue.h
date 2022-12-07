#ifndef MILKYWAY_AUDIO_DOWNLOAD_MSG_QUEUE_H
#define MILKYWAY_AUDIO_DOWNLOAD_MSG_QUEUE_H

#include "MsgQueue.h"

enum class EAudioFileEvent {
    DOWNLOAD, DELETE
};

class AudioFileMsgData {
public:
    EAudioFileEvent event = EAudioFileEvent::DOWNLOAD;
    long id;
    String filename;
};

class AudioFileMsgQueue : public MsgQueue<AudioFileMsgData> {
public:
    AudioFileMsgQueue(int length) : MsgQueue(length, sizeof(AudioFileMsgData *)) {}
};


#endif //MILKYWAY_AUDIO_DOWNLOAD_MSG_QUEUE_H
