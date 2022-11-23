#include "WebSocketClient.h"
#include "WifiModule.h"

WebServer webServer(80);

void receiveWifi() {
  WifiModule &wifiModule = WifiModule::getInstance();

  if (!webServer.hasArg("plain")) {
    webServer.send(400, "text/plain", "no plainBody");
    return;
  }
  String plainBody = webServer.arg("plain");
  DynamicJsonDocument doc(plainBody.length() * 2);
  deserializeJson(doc, plainBody);

  if (doc.containsKey("ssid") && doc.containsKey("password")) {
    String status = wifiModule.connectWifi(doc["ssid"], doc["password"]);
    Serial.println(status);
    webServer.send(status == "WL_CONNECTED" ? 200 : 403);
  } else {
    webServer.send(400, "text/plain", "wrong json data");
  }
}

void connectSocket() {
  WebSocketClient &client = WebSocketClient::getInstance();

  if (!webServer.hasArg("plain")) {
    webServer.send(400, "text/plain", "no plainBody");
    return;
  }
  String plainBody = webServer.arg("plain");
  DynamicJsonDocument doc(plainBody.length() * 2);
  deserializeJson(doc, plainBody);

  if (doc.containsKey("host") && doc.containsKey("port") && doc.containsKey("withSSL")) {
    client.setHost(doc["host"]);
    client.setPort(doc["port"]);
    client.setWithSsl(doc["withSSL"]);
    client.connect();
    webServer.send(200);
  } else {
    webServer.send(400, "text/plain", "wrong json data");
  }
}

void setup() {
  WiFiClass::mode(WIFI_MODE_STA);

  Serial.begin(115200);

  WifiModule &wifiModule = WifiModule::getInstance();

  wifiModule.setIp("192.168.1.1", "192.168.1.1", "255.255.255.0");
  wifiModule.setApInfo("Kira", "123456789");
  wifiModule.start();

  webServer.on("/wifi", HTTP_POST, &receiveWifi);
  webServer.on("/socket/connect", HTTP_POST, &connectSocket);
  webServer.begin();

  SDCard::getInstance().init();
}

void loop() {
  WebSocketClient &webSocketClient = WebSocketClient::getInstance();

  webServer.handleClient();
  webSocketClient.loop();
}