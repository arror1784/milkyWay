#ifndef MILKYWAY_EEPROMCONTROL_H
#define MILKYWAY_EEPROMCONTROL_H

#include "Singleton.h"
#include <WString.h>
#include <EEPROM.h>

class EepromControl : public Singleton<EepromControl>{

public:

  void init();    
  String getSerial();
  void setSerial(String serial);

  void setWifiPsk(String ssid, String passwd);
  String getWifiSsid();
  String getWifiPsk();

  static const String defaultSerial_;

private:

  static const int eepromSize_;
  static const int serialAddress_;
  static const int wifiSsidAddress_;
  static const int wifiPasswdAddress_;

  static const int serialFlagAddress_;
  static const int wifiFlagAddress_;

  static const int flagValue_;

  bool _isSerialSet = false;
  bool _isWifiSet = false;

};


#endif //MILKYWAY_EEPROMCONTROL_H