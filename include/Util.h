#ifndef ESP32_LIB_UTIL_H
#define ESP32_LIB_UTIL_H

#include <string>
#include <vector>
#include <WString.h>
#include <IPAddress.h>

class Util {
public:
  static String ipToString(IPAddress ip);
  static IPAddress stringToIp(const String &ip);
  static std::vector<String> stringSplit(const String &str, char Delimiter);
};


#endif //ESP32_LIB_UTIL_H
