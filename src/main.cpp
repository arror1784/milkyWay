#include <WebServer.h>
#include <ArduinoJson.h>
#include <FFat.h>
// #include <Arduino_FreeRTOS.h>
#include<sstream>

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

#define SOCKET_SUCESS_BIT   (1UL << 0UL) // zero shift for bit0
#define SOCKET_FAIL_BIT   (1UL << 1UL) // 1 shift for flag  bit 1

static const String host = "192.168.219.112";
static const int port = 6001;
static const bool ssl = false;

NeoPixelMsgQueue neoPixelMsgQueue(5);
AudioControl audioControl(I2S_LRC,I2S_BCLK,I2S_DOUT);
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

AudioMsgQueue audioMsgQueue(5);
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

WebSocketClient wsClient;
WebServer webServer(80);
EventGroupHandle_t xCreatedEventGroup;

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

    SDUtil::writeFile(SDUtil::wifiInfoPath_,ssid + " " + password);

    String status = WifiModule::getInstance().connectWifi(doc["ssid"], doc["password"]);

    if(status != "WL_CONNECTED"){
      webServer.send(400, "text/plain", "network connect fail");
      return;
    }

    WifiModule::getInstance().stop();
    wsClient.connect();
    webServer.send(200);
  } else {
    webServer.send(400, "text/plain", "wrong json data");
  }
}
void setup() {
  Serial.begin(115200);

  xCreatedEventGroup = xEventGroupCreate();
  if(xCreatedEventGroup == NULL){
    Serial.println("event group create Fail");
  }

  SDUtil::getInstance().init();
  WiFiClass::mode(WIFI_MODE_STA);
  Serial.println(SDUtil::getInstance().getSerial());
  WifiModule::getInstance().setIp("192.168.1.1", "192.168.1.1", "255.255.255.0");
  WifiModule::getInstance().setApInfo(SDUtil::getInstance().getSerial());

  wsClient.setHost(host);
  wsClient.setPort(port);
  wsClient.setWithSsl(ssl);
  wsClient.onConnected([&](uint8_t *payload, size_t length){

    Serial.println("websocket connected");

    DynamicJsonDocument doc(512);
    JsonObject json = doc.to<JsonObject>();
    json["event"] = "registerDeviceSession";
    json["name"] = "Kira";
    json["type"] = "Mirror";

    String strJson;
    serializeJson(json, strJson);

    wsClient.sendText(strJson);

  });
  wsClient.onDisconnected([&](uint8_t *payload, size_t length){
    Serial.println("websocket disconnected");
    WifiModule::getInstance().start();
  });
  wsClient.onTextMessageReceived([&](uint8_t *payload, size_t length){
      Serial.println(String(payload,length));
      
      DynamicJsonDocument doc(length * 2);
      deserializeJson(doc, payload);

      if (doc.containsKey("authenticationToken")) {
        SDUtil::authenticationToken_ = String(doc["authenticationToken"]);
        WebSocketClient::parsePlayList(doc["playlist"]);
      }
      else if (doc["event"] == "SendLightEffect") {
      }
      else if (doc["event"] == "SendSound") {
        String protocol = ssl ? "https://" : "http://";
        long id = doc["id"];
        String filename = doc["filename"];
        String url = protocol + host + ":" + port + "/api/file/" + filename;

        SDUtil::downloadFile(url, id, filename);
      }
      else if (doc["event"] == "SendPlaylist") {
        WebSocketClient::parsePlayList(doc["data"]);
      }
  });
  wsClient.onErrorReceived([&](uint8_t *payload, size_t length){
    Serial.println(String(payload,length));
    Serial.println("websocket errorReceived");
  });

  String str = SDUtil::readFile(SDUtil::wifiInfoPath_);
  Serial.println(str);
  std::string s(str.c_str(),str.length());
  std::istringstream ss(s);
  std::string stringBuffer;
  std::vector<std::string> x;
  x.clear();

  while (getline(ss, stringBuffer, ' ')){
    x.push_back(stringBuffer);
  }
  if(x.size() == 2){
    
    String status = WifiModule::getInstance().connectWifi(x[0].c_str(),x[1].c_str());
    if(status != "WL_CONNECTED"){
      Serial.println("wifi connect fail");
      WifiModule::getInstance().start();
    }else{
      Serial.println("wifi connect");
      wsClient.connect();
    }
  }else{
    Serial.println("there is no wifi passwd");
    WifiModule::getInstance().start();
  }

  webServer.on("/wifi", HTTP_POST, &receiveWifi);
  webServer.begin();

  xTaskCreate(neoPixelTask,"neoPixelTask",10000,NULL,0,NULL);
  xTaskCreate(audioTask,"audioTask",50000,NULL,0,NULL);
}

void loop() {
  webServer.handleClient();
  wsClient.loop();
}

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}