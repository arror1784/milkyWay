#include <WebServer.h>
#include <ArduinoJson.h>
#include <FFat.h>
#include <sstream>

#include "WebSocketClient.h"
#include "WifiModule.h"
#include "SDUtil.h"
#include "SEN0153.h"

static const String host = "192.168.219.112";
static const int port = 6001;
static const bool ssl = false;

static const uint8_t send0153Address =  SEN0153_ADDRESS_DEFAULT;
static const int8_t sen0153RX = 16;
static const int8_t sen0153TX = 17;

WebSocketClient wsClient;
WebServer webServer(80);
SEN0153 ult(sen0153RX, sen0153TX, SEN0153_BAUD_RATE_DEFAULT);

int distance = 0;

void receiveWifi() {
    if (!webServer.hasArg("plain")) {
        webServer.send(400, "text/plain", "no plainBody");
        return;
    }
    String plainBody = webServer.arg("plain");
    DynamicJsonDocument doc(plainBody.length() * 2);
    deserializeJson(doc, plainBody);

    if (doc.containsKey("ssid") && doc.containsKey("password")) {

        String ssid = doc["ssid"];
        String password = doc["password"];

        SDUtil::writeFile(SDUtil::wifiInfoPath_, ssid + " " + password);

        String status = WifiModule::getInstance().connectWifi(doc["ssid"], doc["password"]);

        if (status != "WL_CONNECTED") {
            webServer.send(400, "text/plain", "network connect fail");
            return;
        }

        WifiModule::getInstance().stop();
        wsClient.connect();
        webServer.send(200);
    }
    else {
        webServer.send(400, "text/plain", "wrong json data");
    }
}

void setup() {
    Serial.begin(115200);

    SDUtil::getInstance().init();
    WiFiClass::mode(WIFI_MODE_STA);
    Serial.println(SDUtil::getInstance().getSerial());
    WifiModule::getInstance().setIp("192.168.1.1", "192.168.1.1", "255.255.255.0");
    WifiModule::getInstance().setApInfo(SDUtil::getInstance().getSerial());

    wsClient.setHost(host);
    wsClient.setPort(port);
    wsClient.setWithSsl(ssl);
    wsClient.onConnected([&](uint8_t *payload, size_t length) {

        Serial.println("websocket connected");

        DynamicJsonDocument doc(512);
        JsonObject json = doc.to<JsonObject>();
        json["event"] = "registerDeviceSession";
        json["name"] = "Kira";
        json["type"] = "HumanDetection";

        String strJson;
        serializeJson(json, strJson);

        wsClient.sendText(strJson);

    });
    wsClient.onDisconnected([&](uint8_t *payload, size_t length) {
        Serial.println("websocket disconnected");
        WifiModule::getInstance().start();
    });
    wsClient.onTextMessageReceived([&](uint8_t *payload, size_t length) {
        Serial.println(String(payload, length));

        DynamicJsonDocument doc(length * 2);
        deserializeJson(doc, payload);

        if (doc.containsKey("sens")) {

        }
    });
    wsClient.onErrorReceived([&](uint8_t *payload, size_t length) {
        Serial.println(String(payload, length));
        Serial.println("websocket errorReceived");
    });

    String str = SDUtil::readFile(SDUtil::wifiInfoPath_);
    Serial.println(str);
    std::string s(str.c_str(), str.length());
    std::istringstream ss(s);
    std::string stringBuffer;
    std::vector<std::string> x;
    x.clear();

    while (getline(ss, stringBuffer, ' ')) {
        x.push_back(stringBuffer);
    }
    if (x.size() == 2) {
        String status = WifiModule::getInstance().connectWifi(x[0].c_str(), x[1].c_str());
        if (status != "WL_CONNECTED") {
            Serial.println("wifi connect fail");
            WifiModule::getInstance().start();
        }
        else {
            Serial.println("wifi connect");
            wsClient.connect();
        }
    }
    else {
        Serial.println("there is no wifi passwd");
        WifiModule::getInstance().start();
    }

    webServer.on("/wifi", HTTP_POST, &receiveWifi);
    webServer.begin();
}

void loop() {
    webServer.handleClient();
    wsClient.loop();

    uint16_t distance = ult.readDistance(send0153Address);
    if () {

    }
}