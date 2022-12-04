#include "MsgQueues.h"

AudioMsgQueue MsgQueues::audioMsgQueue_(5);
ShuffleMsgQueue MsgQueues::shuffleMsgQueue_(5);
NeoPixelMsgQueue MsgQueues::neoPixelMsgQueue_(5);
