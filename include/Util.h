#ifndef ESP32_LIB_UTIL_H
#define ESP32_LIB_UTIL_H

#include <string>
#include <sstream>
#include <vector>

#include <FS.h>
#include <WString.h>
#include <IPAddress.h>

enum class ELightMode {
  Breathing = 0, Blinking = 1, ColorChange = 3, Mixed, Sync, N
};

enum class EDeviceType {
  Mirror, HumanDetection, N
};

enum class EInteractionMode {
  LightOnly, SoundOnly, Shuffle, Synchronization, N
};

enum class EOperationMode {
  Default, HumanDetectionA, HumanDetectionB, N
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
