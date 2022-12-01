#include <SD.h>
#include <SD_MMC.h>
#include <FFat.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>

#include "ArduiKalman.h"
#include "WifiModule.h"
#include "SEN0153.h"

static const String mirrorSSID = "Kira";
static const String ssid = "Kira humanDetection";

static const uint8_t send0153Address = SEN0153_ADDRESS_DEFAULT;
static const int8_t sen0153RX = 16;
static const int8_t sen0153TX = 17;

WebServer webServer(80);
SEN0153 ult(sen0153RX, sen0153TX, SEN0153_BAUD_RATE_DEFAULT);
uint16_t distance = 0;
int minimum = 30;
int maximum = 450;


bool getIsDetected(uint16_t distance) {
    return true;
}

void setup() {
    Serial.begin(19200);

    while (1) {
        String status = WifiModule::getInstance().connectWifi(mirrorSSID);
        if (status != "WL_CONNECTED") {
            Serial.println("wifi connect fail");
        }
        else {
            Serial.println("wifi connect");
            break;
        }
    }

    ult.begin();
}

void loop() {
    webServer.handleClient();

    distance = ult.readDistance(send0153Address);
    Serial.println(String("distance : ") + distance);
    if (distance > minimum && distance < maximum) {

//        bool isDetected = getIsDetected(distance);
//
//        if(isDetected) {
//
//        }
    }
    delay(50);
}