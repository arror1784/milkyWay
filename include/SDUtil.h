#ifndef MILKYWAY_SDUTIL_H
#define MILKYWAY_SDUTIL_H

#include <WString.h>

#include "Singleton.h"
class SDUtil : public Singleton<SDUtil>{

public:
  void init();

  String getSerial();

  static bool downloadFile(const String &api, int id, const String &filename);
  static bool writeFile(const String &path,const String &data);
  static String readFile(const String &path);
  static bool  exists(const String &path);

  static String authenticationToken_;

  static const String wifiInfoPath_;
  static const String serialPath_;

private:
  String _serial;

};

#endif //MILKYWAY_SDUTIL_H
