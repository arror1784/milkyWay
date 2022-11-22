#include "Websocket.h"
#include "WifiModule.h"

WebServer webServer(80);
WebSocket webSocket;

void receiveWifi() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "text/plain", "no plainBody");
    return;
  }
  String plainBody = webServer.arg("plain");
  DynamicJsonDocument doc(plainBody.length());
  deserializeJson(doc, plainBody);

  if (doc.containsKey("ssid") && doc.containsKey("password")) {
    String status = WifiModule::getInstance().connectWifi(doc["ssid"], doc["password"]);
    Serial.println(status);
    webServer.send(status == "WL_CONNECTED" ? 200 : 403);
  } else {
    webServer.send(400, "text/plain", "wrong json data");
  }
}

void connectSocket() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "text/plain", "no plainBody");
    return;
  }
  String plainBody = webServer.arg("plain");
  DynamicJsonDocument doc(plainBody.length());
  deserializeJson(doc, plainBody);

  if (doc.containsKey("host") && doc.containsKey("port") && doc.containsKey("withSSL")) {
    webSocket.connect(doc["host"], doc["port"], doc["withSSL"] == "true");
    int isConnected = webSocket.isConnected();
    Serial.println(isConnected ? "WebSocket Connected!" : "WebSocket Connect Faild!");
    webServer.send(isConnected ? 200 : 403);
  } else {
    webServer.send(400, "text/plain", "wrong json data");
  }
}

void setup() {
  WiFiClass::mode(WIFI_MODE_APSTA);

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