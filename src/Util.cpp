#include "Util.h"

String Util::ipToString(IPAddress ip) {
    String s = "";
    for (int i = 0; i < 4; i++) {
        s += i ? "." + String(ip[i]) : String(ip[i]);
    }
    return s;
}

IPAddress Util::stringToIp(const String &ip) {
    std::vector<String> splitString = Util::stringSplit(ip, '.');

    uint8_t ip1 = splitString[0].toInt();
    uint8_t ip2 = splitString[1].toInt();
    uint8_t ip3 = splitString[2].toInt();
    uint8_t ip4 = splitString[3].toInt();

    return {ip1, ip2, ip3, ip4};
}

std::vector<String> Util::stringSplit(const String &str, char Delimiter) {
    std::istringstream iss(str.c_str());
    std::string buffer;

    std::vector<String> result;

    while (std::getline(iss, buffer, Delimiter)) {
        result.emplace_back(buffer.c_str());
    }

    return result;
}

ELightMode Util::stringToELightMode(const String &string) {
    if (string.compareTo("Breathing") == 0) return ELightMode::Breathing;
    else if (string.compareTo("Blinking") == 0) return ELightMode::Blinking;
    else if (string.compareTo("ColorChange") == 0) return ELightMode::ColorChange;
    return ELightMode::Mixed;
}

EDeviceType Util::stringToEDeviceType(const String &string) {
    if (string.compareTo("Mirror") == 0) return EDeviceType::Mirror;
    return EDeviceType::HumanDetection;
}

EInteractionMode Util::stringToEInteractionMode(const String &string) {
    if (string.compareTo("LightOnly") == 0) return EInteractionMode::LightOnly;
    else if (string.compareTo("SoundOnly") == 0) return EInteractionMode::SoundOnly;
    else if (string.compareTo("PingPong") == 0) return EInteractionMode::PingPong;
    else if (string.compareTo("Synchronization") == 0) return EInteractionMode::Synchronization;
    return EInteractionMode::N;
}

EOperationMode Util::stringToEOperationMode(const String &string) {
    if (string.compareTo("Default") == 0) return EOperationMode::Default;
    else if (string.compareTo("HumanDetectionA") == 0) return EOperationMode::HumanDetectionA;
    else if (string.compareTo("HumanDetectionB") == 0) return EOperationMode::HumanDetectionB;
    return EOperationMode::Default;
}

uint32_t Util::stringToRGBW(const String &string) {
    int red = (int) strtol(string.substring(1, 3).c_str(), nullptr, 16) << 16;
    int green = (int) strtol(string.substring(3, 5).c_str(), nullptr, 16) << 8;
    int blue = (int) strtol(string.substring(5, 7).c_str(), nullptr, 16);
    int white = 0x00 << 24;
    return red + green + blue + white;
}

void Util::taskDelay(const uint32_t time) {

    TickType_t xLastWakeTime = xTaskGetTickCount();

    xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(time));
}
