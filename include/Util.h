#ifndef ESP32_LIB_UTIL_H
#define ESP32_LIB_UTIL_H

#include <string>
#include <sstream>
#include <vector>

#include <FS.h>
#include <WString.h>
#include <IPAddress.h>

class Util {
public:
  static String ipToString(IPAddress ip);

  static IPAddress stringToIp(const String &ip);

  static std::vector<String> stringSplit(const String &str, char Delimiter);

  static void listDir(FS &fs, const char *dirname, uint8_t levels);
};


#endif //ESP32_LIB_UTIL_H
