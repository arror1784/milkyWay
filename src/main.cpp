#include "Websocket.h"
#include "WifiModule.h"

WebServer webServer(80);
WebSocketsClient webSocket;

void receiveWifi() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "text/plain", "no body");
    return;
  }
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, webServer.arg("plain"));

  if (doc.containsKey("ssid") && doc.containsKey("password")) {
    bool isConnected = WifiModule::getInstance().connectWifi(doc["ssid"], doc["password"]);
    Serial.println(isConnected ? "Connected!" : "Connect Faild!");
    webServer.send(isConnected ? 200 : 403);
  } else {
    webServer.send(400, "text/plain", "wrong json data");
  }
}

void connectSocket() {
  webSocket.beginSSL("wss://kira-api.wimcorp.dev/socket", 443);
  int status = webSocket.isConnected();
  webServer.send(200, "text/plain", String("Status Code : ") + String(status));
  webSocket.onEvent([](WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
      case WStype_TEXT:
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        break;
      case WStype_BIN:
        break;
    }
  });
}

void setup() {
  Serial.begin(115200);

  WifiModule::getInstance().setIp("192.168.1.1", "192.168.1.1", "255.255.255.0");
  WifiModule::getInstance().setApInfo("MIRROR-0001-Access-Point", "123456789");
  WifiModule::getInstance().start();

  webServer.on("/wifi", HTTP_POST, &receiveWifi);
  webServer.on("/socket/check", HTTP_GET, &connectSocket);

  webServer.begin();

  bool sdStatus = SD.begin(5);
  if (!sdStatus) {
    while (1);
  }
}

void loop() {
  webServer.handleClient();
}