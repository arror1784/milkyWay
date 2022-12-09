#ifndef MILKYWAY_USERMODECONTROL_H
#define MILKYWAY_USERMODECONTROL_H


#include <Singleton.h>
#include <Util.h>

class UserModeControl : public Singleton<UserModeControl>{
public:
    EInteractionMode interactionMode = EInteractionMode::N;
    EOperationMode operationMode = EOperationMode::N;
    bool humanDetection = false;
};


#endif //MILKYWAY_USERMODECONTROL_H
