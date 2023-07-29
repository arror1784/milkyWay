#include "EepromControl.h"

const int EepromControl::eepromSize_ = 63;
const int EepromControl::serialAddress_ = 0;
const int EepromControl::wifiPasswdAddress_ = 20;
const int EepromControl::wifiSsidAddress_ = 40;
const int EepromControl::serialFlagAddress_ = 60;
const int EepromControl::wifiFlagAddress_ = 61;
const int EepromControl::pingPongStartModeAddress_ = 62;
const int EepromControl::flagValue_ = 0xfe;
const String EepromControl::defaultSerial_ = "KIRA_Mirror_00000";

void EepromControl::init() {
    EEPROM.begin(EepromControl::eepromSize_);

    _isSerialSet = EEPROM.readByte(serialFlagAddress_) == flagValue_;
    _isWifiSet = EEPROM.readByte(wifiFlagAddress_) == flagValue_;

}

String EepromControl::getSerial() {
    if (!_isSerialSet)
        return EepromControl::defaultSerial_;

    char temp[20] = {0};
    EEPROM.readBytes(EepromControl::serialAddress_, temp, 20);
    return {temp};
}

void EepromControl::setSerial(String serial) {
    char temp[20] = {0};
    strncpy(temp, serial.c_str(), serial.length());

    EEPROM.writeBytes(EepromControl::serialAddress_, temp, 20);
    EEPROM.commit();

    EEPROM.writeByte(EepromControl::serialFlagAddress_, EepromControl::flagValue_);
    EEPROM.commit();

    _isSerialSet = true;

}

void EepromControl::setWifiPsk(String ssid, String passwd) {
    char ssidTemp[20] = {0};
    strncpy(ssidTemp, ssid.c_str(), ssid.length());

    EEPROM.writeBytes(EepromControl::wifiSsidAddress_, ssidTemp, 20);
    EEPROM.commit();

    char passwdTemp[20] = {0};
    strncpy(passwdTemp, passwd.c_str(), passwd.length());

    EEPROM.writeBytes(EepromControl::wifiPasswdAddress_, passwdTemp, 20);
    EEPROM.commit();

    EEPROM.writeByte(EepromControl::wifiFlagAddress_, EepromControl::flagValue_);
    EEPROM.commit();

    _isWifiSet = true;
}

String EepromControl::getWifiSsid() const {
    if (!_isWifiSet) return "";

    char temp[20] = {0};
    EEPROM.readBytes(EepromControl::wifiSsidAddress_, temp, 20);
    return String(temp);
}

String EepromControl::getWifiPsk() const {
    if (!_isWifiSet) return "";

    char temp[20] = {0};
    EEPROM.readBytes(EepromControl::wifiPasswdAddress_, temp, 20);
    return String(temp);
}

uint8_t EepromControl::getPingPongStartMode() {
    return EEPROM.readBool(EepromControl::pingPongStartModeAddress_);
}

void EepromControl::setPingPongStartMode(uint8_t mode) {
    EEPROM.writeByte(EepromControl::pingPongStartModeAddress_, mode);
}
