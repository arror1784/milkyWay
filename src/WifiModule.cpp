#include "WifiModule.h"

void WifiModule::start() {
  Serial.println();
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(_localIP, _gateway, _subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(_ssid.c_str(), "kirakira") ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
}

void WifiModule::stop() {
  WiFi.softAPdisconnect(true);
}

String WifiModule::connectWifi(const String &ssid, const String &password) {
  if(password.isEmpty()) {
      WiFi.begin(ssid.c_str());
  } else {
      WiFi.begin(ssid.c_str(), password.c_str());
  }

  for(int j = 0; j < 3 ; j++){
    Serial.println("try connect");
    for (int i = 0; i < 5; i++) {
      if(WiFiClass::status() == WL_CONNECTED) {
        break;
      }
      Serial.println("Connecting to WiFi.." + String(i));
      delay(1000);
    }
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
      Serial.println(WiFi.localIP());
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

void WifiModule::disconnectWifi(){
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

std::vector<ApInfo> WifiModule::getApList(){
  std::vector<ApInfo> list;

  Serial.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println("Couldn't get a WiFi connection");
    return list;
  }

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");
    Serial.println(getEncryptionStr(WiFi.encryptionType(thisNet)).c_str());
    list.push_back({WiFi.SSID(thisNet),WiFi.BSSIDstr(thisNet),WiFi.encryptionType(thisNet)});
  }

  return list;
}
