#include "WifiModule.h"

void WifiModule::start() {
    if (_isOnApMode) return;

    _isOnApMode = true;
    SERIAL_PRINTLN("");
    SERIAL_PRINT("Setting soft-AP configuration ... ");
    SERIAL_PRINTLN(WiFi.softAPConfig(_localIP, _gateway, _subnet) ? "Ready" : "Failed!");

    SERIAL_PRINT("Setting soft-AP ... ");
    SERIAL_PRINTLN(WiFi.softAP(_ssid.c_str(), NULL) ? "Ready" : "Failed!");

    SERIAL_PRINT("Soft-AP IP address = ");
    SERIAL_PRINTLN(WiFi.softAPIP());
}

void WifiModule::stop() {
    WiFi.softAPdisconnect(true);
    _isOnApMode = false;
}

String WifiModule::connectWifi(const String &ssid, const String &password) {
    WiFi.begin(ssid.c_str(), password.c_str());
    
    SERIAL_PRINTLN("try connect");
    for (int i = 0; i < 5; i++) {
        if (WiFiClass::status() == WL_CONNECTED) {
            break;
        }
        SERIAL_PRINTLN("Connecting to WiFi.." + String(i));
        delay(1000);
    }

    switch (WiFiClass::status()) {
        case WL_NO_SHIELD:
            return "WL_NO_SHIELD";
        case WL_IDLE_STATUS:
            return "WL_IDLE_STATUS";
        case WL_NO_SSID_AVAIL:
            return "WL_NO_SSID_AVAIL";
        case WL_SCAN_COMPLETED:
            return "WL_SCAN_COMPLETED";
        case WL_CONNECTED:
            SERIAL_PRINTLN(WiFi.localIP());
            return "WL_CONNECTED";
        case WL_CONNECT_FAILED:
            return "WL_CONNECT_FAILED";
        case WL_CONNECTION_LOST:
            return "WL_CONNECTION_LOST";
        case WL_DISCONNECTED:
            return "WL_DISCONNECTED";
    }
    return "WL_DISCONNECTED";
}

void WifiModule::disconnectWifi() {
    WiFi.disconnect();
}

void WifiModule::setApInfo(const String &ssid) {
    _ssid = ssid;
}

void WifiModule::setIp(const String &localIp, const String &gateway, const String &subnet) {
    _localIP = Util::stringToIp(localIp);
    _gateway = Util::stringToIp(gateway);
    _subnet = Util::stringToIp(subnet);
}

std::vector<ApInfo> WifiModule::getApList() {
    std::vector<ApInfo> list;

    SERIAL_PRINTLN("** Scan Networks **");
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1) {
        SERIAL_PRINTLN("Couldn't get a WiFi connection");
        return list;
    }

    // print the list of networks seen:
    SERIAL_PRINT("number of available networks:");
    SERIAL_PRINTLN(numSsid);

    // print the network number and name for each network found:
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
        SERIAL_PRINT(thisNet);
        SERIAL_PRINT(") ");
        SERIAL_PRINT(WiFi.SSID(thisNet));
        SERIAL_PRINT("\tSignal: ");
        SERIAL_PRINT(WiFi.RSSI(thisNet));
        SERIAL_PRINT(" dBm");
        SERIAL_PRINT("\tEncryption: ");
        SERIAL_PRINTLN(getEncryptionStr(WiFi.encryptionType(thisNet)).c_str());
        list.push_back({WiFi.SSID(thisNet), WiFi.BSSIDstr(thisNet), WiFi.encryptionType(thisNet)});
    }

    return list;
}
