#ifndef MILKYWAY_USERMODECONTROL_H
#define MILKYWAY_USERMODECONTROL_H


#include <Singleton.h>
#include <Util.h>

class UserModeControl : public Singleton<UserModeControl>{
public:
    EInteractionMode interactionMode = EInteractionMode::LightOnly;
    EOperationMode operationMode = EOperationMode::Default;
    bool humanDetection = false;
};


#endif //MILKYWAY_USERMODECONTROL_H
