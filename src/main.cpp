#include <WebServer.h>
#include <ArduinoJson.h>
#include <FFat.h>
// #include <Arduino_FreeRTOS.h>

#include "WebSocketClient.h"
#include "WifiModule.h"
#include "SDUtil.h"
#include "AudioControl.h"
#include "NeoPixel.h"

#include "AudioMsgQueue.h"
#include "NeoPixelMsgQueue.h"

#define LED_PIN 32
#define LED_LENGTH 21

#define I2S_DOUT      2
#define I2S_BCLK      0
#define I2S_LRC       4

AudioMsgQueue audioMsgQueue(5);
NeoPixelMsgQueue neoPixelMsgQueue(5);

WebServer webServer(80);
// WebSocketClient wsClient;
AudioControl audioControl(I2S_LRC,I2S_BCLK,I2S_DOUT);

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
  // WebSocketClient &client = WebSocketClient::getInstance();

  // if (!webServer.hasArg("plain")) {
  //   webServer.send(400, "text/plain", "no plainBody");
  //   return;
  // }
  // String plainBody = webServer.arg("plain");
  // DynamicJsonDocument doc(plainBody.length() * 2);
  // deserializeJson(doc, plainBody);

  // if (doc.containsKey("host") && doc.containsKey("port") && doc.containsKey("withSSL")) {
  //   client.setHost(doc["host"]);
  //   client.setPort(doc["port"]);
  //   client.setWithSsl(doc["withSSL"]);
  //   client.connect();
  //   webServer.send(200);
  // } else {
  //   webServer.send(400, "text/plain", "wrong json data");
  // }
}
void neoPixelTask(void* parms) {

    Neopixel neopixel(LED_LENGTH, LED_PIN, NEO_GRBW | NEO_KHZ800,true);

    while(1){
      NeoPixelMsgData* msg = neoPixelMsgQueue.recv();
      if(msg != nullptr){
      }
      neopixel.dim(10,500);
    }
    vTaskDelete(NULL);

}

void audioTask(void* parms){
  
  audioControl.setVolume(21); // 0...21

  while(1){
    AudioMsgData* msg = audioMsgQueue.recv();
    if(msg != nullptr){
      auto& list = msg->list;
      
      audioControl.setPlayList(list);

      // delete msg;
    }
    audioControl.loop();
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

  String status = wifiModule.connectWifi("301_main_2.4", "hongsamcoffee3*");

  webServer.on("/wifi", HTTP_POST, &receiveWifi);
  webServer.on("/socket/connect", HTTP_POST, &connectSocket);

  SDUtil::init();
  // SDUtil::writeFile();
  webServer.begin();

  xTaskCreate(neoPixelTask,"neoPixelTask",10000,NULL,0,NULL);
  xTaskCreate(audioTask,"audioTask",50000,NULL,0,NULL);
  
  Sound snd;
  snd.filename = "/test.mp3";
  snd.size = 1111;

  AudioMsgData* data = new AudioMsgData();

  data->list.sounds.push_back(snd);
  data->list.id = 0000;
  data->list.isShuffle = false;

  data->events = AudioMQEvents::UPDATE_MODE;

  Serial.println((int)data);
  audioMsgQueue.send(data);

  // vTaskDelay(pdMS_TO_TICKS(5000));
  // delete data;
  

}

void loop() {
  webServer.handleClient();
  // WebSocketClient::getInstance().loop();

}

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}