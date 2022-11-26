#include <WebServer.h>
#include <ArduinoJson.h>
#include <FFat.h>
// #include <Arduino_FreeRTOS.h>

#include "WebSocketClient.h"
#include "WifiModule.h"
#include "SDUtil.h"
#include "SimpleTimer.h"
#include "Audio.h"

#define LED_PIN 32
#define LED_LENGTH 21

#define I2S_DOUT      2
#define I2S_BCLK      0
#define I2S_LRC       4

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

void neoPixelTask(void* parms) {

    Neopixel neopixel(LED_LENGTH, LED_PIN, NEO_GRBW | NEO_KHZ800,true);
    std::vector<uint32_t> preset;
    for(int i = 0;i < LED_LENGTH; i++){
      preset.push_back(0xffffff);
    }
    neopixel.pushColorPreset(preset);

    while(1){
      neopixel.dim(10,500);

    }
    vTaskDelete(NULL);

}

void audioTask(void* parms){

  Audio audio;

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21); // 0...21
  
  audio.connecttoFS(SD, "/test.mp3");

  while(1){
    audio.loop();
  }
  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);

  WiFiClass::mode(WIFI_MODE_STA);
  WifiModule &wifiModule = WifiModule::getInstance();

  wifiModule.setIp("192.168.1.1", "192.168.1.1", "255.255.255.0");
  wifiModule.setApInfo("Kira", "123456789");
  wifiModule.start();

  // String status = wifiModule.connectWifi("Wim", "Wim12345!");

  webServer.on("/wifi", HTTP_POST, &receiveWifi);
  webServer.on("/socket/connect", HTTP_POST, &connectSocket);

  SDUtil::init();

  // 네오픽셀 실행용 엔드포인트
  webServer.on("/led", HTTP_POST, [=]() {
    webServer.send(200);
  });

  webServer.begin();

  TaskHandle_t xNeoPixelHandle = NULL;
  TaskHandle_t xAudioHandle = NULL;

  xTaskCreate(neoPixelTask,"neoPixelTask",10000,NULL,0,&xNeoPixelHandle);
  xTaskCreate(audioTask,"audioTask",100000,NULL,0,&xAudioHandle);

}

void loop() {
  timer.run();
  webServer.handleClient();
  WebSocketClient::getInstance().loop();
}