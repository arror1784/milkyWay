#include <WebServer.h>
#include <ArduinoJson.h>
#include <FFat.h>

#include "WebSocketClient.h"
#include "WifiModule.h"
#include "SDUtil.h"
#include "SimpleTimer.h"

WebServer webServer(80);
SimpleTimer timer;

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

void runNeoPixel() {
  NeoPixel &neoPixel = NeoPixel::getInstance();

  if (!neoPixel.isCurrentLightEffectValid()) {
    return;
  }

  auto lightEffect = neoPixel.getCurrentLightEffect();

  if (lightEffect.mode == ELightMode::Blinking) {
    if (neoPixel.isOn()) {
      neoPixel.off();
    } else {
      neoPixel.on();
    }
  } else if (lightEffect.mode == ELightMode::Breathing) {
    neoPixel.setIsColorChange(false);
    if (neoPixel.getDimmingStatus() == EDimmingStatus::UP) {
      neoPixel.increaseBrightness();
    } else if (neoPixel.getDimmingStatus() == EDimmingStatus::DOWN) {
      neoPixel.lowerBrightness();
    }
  } else if (lightEffect.mode == ELightMode::ColorChange) {
    neoPixel.setIsColorChange(true);
    if (neoPixel.getDimmingStatus() == EDimmingStatus::UP) {
      neoPixel.increaseBrightness();
    } else if (neoPixel.getDimmingStatus() == EDimmingStatus::DOWN) {
      neoPixel.lowerBrightness();
    }
  } else if (lightEffect.mode == ELightMode::Mixed) {

  }
}

void setup() {
  Serial.begin(115200);

  WiFiClass::mode(WIFI_MODE_STA);
  WifiModule &wifiModule = WifiModule::getInstance();

  wifiModule.setIp("192.168.1.1", "192.168.1.1", "255.255.255.0");
  wifiModule.setApInfo("Kira", "123456789");
  wifiModule.start();

  String status = wifiModule.connectWifi("Wim", "Wim12345!");

  webServer.on("/wifi", HTTP_POST, &receiveWifi);
  webServer.on("/socket/connect", HTTP_POST, &connectSocket);

  SDUtil::init();

  // 네오픽셀 실행용 엔드포인트
  webServer.on("/led", HTTP_POST, [=]() {
    timer.setInterval(30, runNeoPixel);
    webServer.send(200);
  });

  webServer.begin();

  NeoPixel::setPin(32);
  NeoPixel::setLedCount(20);

  NeoPixel &neoPixel = NeoPixel::getInstance();

  neoPixel.off();
}

void loop() {
  timer.run();
  webServer.handleClient();
  WebSocketClient::getInstance().loop();
}