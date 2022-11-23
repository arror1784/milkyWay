//
// Created by jepanglee on 2022-10-03.
//

#ifndef MILKYWAY_MDNSMODULE_H
#define MILKYWAY_MDNSMODULE_H

#include <string>
#include <mdns.h>
#include <Arduino.h>

#include "Singleton.h"

class MdnsModule : public Singleton<MdnsModule> {

public:
  MdnsModule();

  ~MdnsModule();

  void mDnsInit(std::string name);

private:
  std::string _hostName;

};


#endif //MILKYWAY_MDNSMODULE_H
