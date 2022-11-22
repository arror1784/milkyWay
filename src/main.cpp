#include "WebSocketClient.h"
#include "WifiModule.h"

WebServer webServer(80);

void receiveWifi() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "text/plain", "no plainBody");
    return;
  }
  String plainBody = webServer.arg("plain");
  DynamicJsonDocument doc(plainBody.length() * 2);
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
  DynamicJsonDocument doc(plainBody.length() * 2);
  deserializeJson(doc, plainBody);

  if (doc.containsKey("host") && doc.containsKey("port") && doc.containsKey("withSSL")) {
    Serial.print(bool(doc["withSSL"]));
    WebSocketClient::getInstance().connect(doc["host"], doc["port"], doc["withSSL"] == "true");
    webServer.send(200);
  } else {
    webServer.send(400, "text/plain", "wrong json data");
  }
}

void setup() {
  WiFiClass::mode(WIFI_MODE_STA);

  Serial.begin(115200);

  WifiModule::getInstance().setIp("192.168.1.1", "192.168.1.1", "255.255.255.0");
  WifiModule::getInstance().setApInfo("MIRROR-0001-Access-Point", "123456789");
  WifiModule::getInstance().start();

  webServer.on("/wifi", HTTP_POST, &receiveWifi);
  webServer.on("/socket/check", HTTP_POST, &connectSocket);

  webServer.begin();

  SDCard::getInstance().init();
}

void loop() {
  webServer.handleClient();
  WebSocketClient::getInstance().loop();
}