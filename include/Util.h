#ifndef ESP32_LIB_UTIL_H
#define ESP32_LIB_UTIL_H

#include <string>
#include <sstream>
#include <vector>

#include <FS.h>
#include <WString.h>
#include <IPAddress.h>

enum class ELightMode {
    Breathing, Blinking, ColorChange, Mixed, None
};

enum class EDeviceType {
    Mirror, HumanDetection, N
};

enum class EInteractionMode {
    LightOnly, SoundOnly, PingPong, Synchronization, N
};

enum class EOperationMode {
    Default, HumanDetectionA, HumanDetectionB, N
};

class Util {
public:
    static String ipToString(IPAddress ip);

    static IPAddress stringToIp(const String &ip);

    static std::vector<String> stringSplit(const String &str, char Delimiter);

    static ELightMode stringToELightMode(const String &string);

    static EDeviceType stringToEDeviceType(const String &string);

    static EInteractionMode stringToEInteractionMode(const String &string);

    static EOperationMode stringToEOperationMode(const String &string);

    static uint32_t stringToRGBW(const String &string);

    static void taskDelay(const uint32_t time);

    static String decodeUrl(String str) {

        String encodedString = "";
        char c;
        char code0;
        char code1;
        for (int i = 0; i < str.length(); i++) {
            c = str.charAt(i);
            if (c == '+') {
                encodedString += ' ';
            }
            else if (c == '%') {
                i++;
                code0 = str.charAt(i);
                i++;
                code1 = str.charAt(i);
                c = (h2int(code0) << 4) | h2int(code1);
                encodedString += c;
            }
            else {

                encodedString += c;
            }

            yield();
        }

        return encodedString;
    }

    static String encodeUrl(String str) {
        const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

        String escaped = "";


        for (char c: str) {
            if (unreserved.find_first_of(c) != std::string::npos) {
                escaped += c;
            }
            else {
                escaped += '%';
                char buf[3];

                sprintf(buf, "%.2X", (unsigned char) c);
                escaped += buf;
            }
        }
        return escaped;
    }

    static unsigned char h2int(char c) {
        if (c >= '0' && c <= '9') {
            return ((unsigned char) c - '0');
        }
        if (c >= 'a' && c <= 'f') {
            return ((unsigned char) c - 'a' + 10);
        }
        if (c >= 'A' && c <= 'F') {
            return ((unsigned char) c - 'A' + 10);
        }
        return (0);
    }

};


#endif //ESP32_LIB_UTIL_H
