#ifndef MILKYWAY_SDUTIL_H
#define MILKYWAY_SDUTIL_H

#include <WString.h>
#include <SD.h>
#include <FS.h>
#include <HTTPClient.h>

#include "Singleton.h"
#include "Util.h"

class SDUtil : public Singleton<SDUtil>{

public:
  void init();

  static bool downloadFile(const String &api, int id, const String &filename);
  static bool writeFile(const String &path,const String &data);
  static String readFile(const String &path);
  static bool  exists(const String &path);
  static void listDir(FS &fs, const char *dirname, uint8_t levels);

  static String authenticationToken_;

  static const String defaultColorSetsPath_;

private:
  String _serial;

};

#endif //MILKYWAY_SDUTIL_H
