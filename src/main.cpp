#include <SD.h>
#include <SD_MMC.h>
#include <FFat.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "ArduiKalman.h"
#include "WifiModule.h"
#include "SEN0153.h"
#include "WebSocketClient.h"

static const String mirrorSSID = "Kira";
static const String ssid = "Kira humanDetection";

static const uint8_t send0153Address = SEN0153_ADDRESS_DEFAULT;
static const int8_t sen0153RX = 16;
static const int8_t sen0153TX = 17;

static const String host = "192.168.219.102";
static const int port = 6001;
static const bool ssl = false;

//static const String host = "kira-api.wimcorp.dev";
//static const int port = 443;
//static const bool ssl = true;

WebSocketClient wsClient;
WebServer webServer(80);
SEN0153 ult(sen0153RX, sen0153TX, SEN0153_BAUD_RATE_DEFAULT);

uint16_t distance = 0;
int minimum = 30;
int maximum = 500;
int sensLow = 80;
int sensHigh = 100;
bool isDetected = false;

bool connectWifiByFlash() {
    // TODO: 플래시에서 와이파이 데이터 가져오기
    String status = WifiModule::getInstance().connectWifi("", "");

    if (status != "WL_CONNECTED") {
        Serial.println("wifi connect fail");
        WifiModule::getInstance().start();
        return false;
    }

    Serial.println("wifi connect");
    WifiModule::getInstance().stop();
    wsClient.connect();
    return true;
}

void receiveWifi() {
    if (!webServer.hasArg("plain")) {
        webServer.send(400, "text/plain", "no plainBody");
        return;
    }
    String plainBody = webServer.arg("plain");
    DynamicJsonDocument doc(plainBody.length() * 2);
    deserializeJson(doc, plainBody);

    if (doc.containsKey("ssid") && doc.containsKey("password")) {
        String strPayload;

        serializeJson(doc, strPayload);

        // TODO : API로 받은 와이파이 정보 플래쉬에 저장하기

        bool status = connectWifiByFlash();

        if (!status) {
            webServer.send(400, "text/plain", "network connect fail");
            return;
        }
        webServer.send(200);
    }
    else {
        webServer.send(400, "text/plain", "wrong json data");
    }
}

bool getIsDetected(uint16_t distance) {
    return distance > sensLow && distance < sensHigh;
}

void setup() {
    Serial.begin(19200);

    WiFiClass::mode(WIFI_MODE_STA);
    WifiModule::getInstance().setIp("192.168.0.1", "192.168.0.1", "255.255.255.0");
    // TODO : 플래시에서 시리얼 데이터 가져오기
    WifiModule::getInstance().setApInfo("");

    wsClient.setHost(host);
    wsClient.setPort(port);
    wsClient.setWithSsl(ssl);
    wsClient.onConnected([&](uint8_t *payload, size_t length) {
        Serial.println("websocket connected");

        DynamicJsonDocument doc(512);
        JsonObject json = doc.to<JsonObject>();
        json["event"] = "registerDeviceSession";
        // TODO : 플래시에서 시리얼 데이터 가져오기
        json["name"] = "";
        Serial.println(String(json["name"]));
        json["type"] = "HumanDetection";

        String strJson;
        serializeJson(json, strJson);

        wsClient.sendText(strJson);
    });
    wsClient.onDisconnected([&](uint8_t *payload, size_t length) {
        Serial.println("websocket disconnected");
    });
    wsClient.onTextMessageReceived([&](uint8_t *payload, size_t length) {
        DynamicJsonDocument doc(length * 2);
        deserializeJson(doc, payload, length);

        if (doc["event"] != "Ping") {
            String strJson;
            serializeJson(doc, strJson);

            Serial.println(strJson);
        }

        if (doc.containsKey("sens")) {
            sensLow = 0;
            sensHigh = doc["sens"];
        }
        else if (doc["event"] == "Ping") {
            DynamicJsonDocument json(512);
            json["event"] = "Pong";

            String strJson;
            serializeJson(json, strJson);

            wsClient.sendText(strJson);
        }
    });
    wsClient.onErrorReceived([&](uint8_t *payload, size_t length) {
        Serial.println(String(payload, length));
        Serial.println("websocket errorReceived");
    });

    webServer.on("/wifi", HTTP_POST, &receiveWifi);
    webServer.begin();

    ult.begin();
}

void loop() {
    webServer.handleClient();
    wsClient.loop();

    // TODO: 큐를 임베디드 기기가 아닌 서버에서 구현하고, 이를 패치해 오는 방식을 바꾸는 것도 고려

    if (WifiModule::getInstance().isConnectedST()) {
        distance = ult.readDistance(send0153Address);
        if (distance > minimum && distance < maximum && isDetected != getIsDetected(distance)) {
            isDetected = !isDetected;

            DynamicJsonDocument doc(512);
            JsonObject json = doc.to<JsonObject>();
            json["event"] = "SendHumanDetection";
            json["data"]["isDetected"] = isDetected;

            String strJson;
            serializeJson(json, strJson);

            wsClient.sendText(strJson);
        }
    }
    else {
        connectWifiByFlash();
    }
    delay(50);
}