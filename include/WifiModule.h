#ifndef MILKYWAY_WIFIMODULE_H
#define MILKYWAY_WIFIMODULE_H

#include <vector>
#include <string>
#include <esp_wifi.h>
#include <WString.h>
#include <WiFi.h>
#include <IPAddress.h>

#include "Singleton.h"
#include "Util.h"

struct ApInfo {
    String ssid;
    String bssid;
    wifi_auth_mode_t encryptionType;
};

class WifiModule : public Singleton<WifiModule> {

public:
    void start();

    void stop();

    String connectWifi(const String &ssid, const String &password);

    void disconnectWifi();

    void setApInfo(const String &ssid);

    bool isConnectedST() const { return WiFi.isConnected(); }

    void setIp(const String &localIp, const String &gateway, const String &subnet);

    std::vector<ApInfo> getApList();

    IPAddress localIP() { return WiFi.localIP(); }

    IPAddress gateway() { return WiFi.gatewayIP(); }

    IPAddress subnet() { return WiFi.subnetMask(); }

    static std::string getEncryptionStr(wifi_auth_mode_t encryptionType) {
        switch (encryptionType) {
            case WIFI_AUTH_OPEN:
                return "WIFI_AUTH_OPEN";
            case WIFI_AUTH_WEP:
                return "WIFI_AUTH_WEP";
            case WIFI_AUTH_WPA_PSK:
                return "WIFI_AUTH_WPA_PSK";
            case WIFI_AUTH_WPA2_PSK:
                return "WIFI_AUTH_WPA2_PSK";
            case WIFI_AUTH_WPA_WPA2_PSK:
                return "WIFI_AUTH_WPA_WPA2_PSK";
            case WIFI_AUTH_WPA2_ENTERPRISE:
                return "WIFI_AUTH_WPA2_ENTERPRISE";
            case WIFI_AUTH_WPA3_PSK:
                return "WIFI_AUTH_WPA3_PSK";
            case WIFI_AUTH_WPA2_WPA3_PSK:
                return "WIFI_AUTH_WPA2_WPA3_PSK";
            case WIFI_AUTH_WAPI_PSK:
                return "WIFI_AUTH_WAPI_PSK";
            case WIFI_AUTH_MAX:
                return "WIFI_AUTH_MAX";
        }
        return "";
    }

private:
    String _ssid;

    IPAddress _localIP;
    IPAddress _gateway;
    IPAddress _subnet;

    bool _isOnApMode = false;
};


#endif //MILKYWAY_WIFIMODULE_H
