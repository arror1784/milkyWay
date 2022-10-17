//
// Created by jepanglee on 2022-10-03.
//

#ifndef MILKYWAY_MDNSMODULE_H
#define MILKYWAY_MDNSMODULE_H

#include "Singleton.h"

#include "string"

class MdnsModule : public Singleton<MdnsModule>{

public:
  MdnsModule();
  ~MdnsModule();

  void mDnsInit(std::string name);

private:
  std::string _hostName;

};


#endif //MILKYWAY_MDNSMODULE_H
