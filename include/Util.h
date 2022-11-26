#ifndef ESP32_LIB_UTIL_H
#define ESP32_LIB_UTIL_H

#include <string>
#include <sstream>
#include <vector>

#include <FS.h>
#include <WString.h>
#include <IPAddress.h>

enum ELightMode {
  Breathing, Blinking, ColorChange, Mixed
};

enum EDeviceType {
  Mirror, HumanDetection
};

enum EInteractionMode {
  LightOnly, SoundOnly, Shuffle, Synchronization
};

enum EOperationMode {
  Default, HumanDetectionA, HumanDetectionB
};

class Util {
public:
  static String ipToString(IPAddress ip);

  static IPAddress stringToIp(const String &ip);

  static std::vector<String> stringSplit(const String &str, char Delimiter);

  static void listDir(FS &fs, const char *dirname, uint8_t levels);

  static ELightMode stringToELightMode(const String &string);

  static EDeviceType stringToEDeviceType(const String &string);

  static EInteractionMode stringToEInteractionMode(const String &string);

  static EOperationMode stringToEOperationMode(const String &string);

  static uint32_t stringToRGBW(const String &string);
};


#endif //ESP32_LIB_UTIL_H
