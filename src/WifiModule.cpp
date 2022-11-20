//
// Created by jepanglee on 2022-10-03.
//
#include <Arduino.h>

#include "WifiModule.h"

#include "WiFi.h"

#include "Util.h"

void WifiModule::start() {
  WiFi.mode(WIFI_MODE_APSTA);
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(_localIP, _gateway, _subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(_ssid.c_str(), _password.c_str()) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
}

void WifiModule::stopApMode() {
  WiFi.softAPdisconnect(true);
}

void WifiModule::connectWifi(const String &ssid, const String &password) {
  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.println("try to connect!");
}

void WifiModule::disconnectWifi(){
  WiFi.disconnect();
}

void WifiModule::setApInfo(const String &ssid, const String &password) {
  _ssid = ssid;
  _password = password;
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
