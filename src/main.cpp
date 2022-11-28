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
#include "UserModeControl.h"

#include "AudioMsgQueue.h"
#include "NeoPixelMsgQueue.h"

#define LED_PIN 32
#define LED_LENGTH 21

#define I2S_DOUT      2
#define I2S_BCLK      0
#define I2S_LRC       4

#define SOCKET_SUCESS_BIT   (1UL << 0UL) // zero shift for bit0
#define SOCKET_FAIL_BIT   (1UL << 1UL) // 1 shift for flag  bit 1

#define SUFFLE_TIMEOUT 5500

static const String host = "192.168.219.112";
static const int port = 6001;
static const bool ssl = false;
static const int syncUpdateResolution = 10;

void processUserMode(JsonObject data);
void processPlayList(JsonObject data);
void processLightEffects(JsonArray array);

NeoPixelMsgQueue neoPixelMsgQueue(5);
void neoPixelTask(void* parms) {

    Neopixel neopixel(LED_LENGTH, LED_PIN, NEO_GRBW | NEO_KHZ800);

    while(1){
      NeoPixelMsgData* msg = neoPixelMsgQueue.recv();
      if(msg != nullptr){
        if(msg->events == NeoPixelMQEvents::UPDATE_EFFECT){
          neopixel.setLightEffects(msg->list);
        }else if(msg->events == NeoPixelMQEvents::UPDATE_MODE){
          neopixel.changeMode(msg->mode);
        }else if(msg->events == NeoPixelMQEvents::UPDATE_ENABLE){
          neopixel.setEnable(msg->enable);
        }else if(msg->events == NeoPixelMQEvents::UPDATE_SYNC){
          neopixel.sync(msg->sync);
        }

        delete msg;
      }
      neopixel.loop();
    }

    vTaskDelete(NULL);
}

AudioControl audioControl(I2S_LRC,I2S_BCLK,I2S_DOUT);
AudioMsgQueue audioMsgQueue(5);

void audioTask(void* parms){
  audioControl.setVolume(21); // 0...21
  TickType_t tick = xTaskGetTickCount();

  while(1){
    AudioMsgData* msg = audioMsgQueue.recv();
    if(msg != nullptr){

      if(msg->events == AudioMQEvents::UPDATE_PLAYLIST){
        Serial.println("updatePLAYLIST");   
        auto& list = msg->list;
        audioControl.setPlayList(list);
        audioControl.playNext();
      }else if(msg->events == AudioMQEvents::UPDATE_ENABLE){
        if(msg->enable)
          audioControl.resume();
        else
          audioControl.pause();
      }

      delete msg;
    }
    audioControl.loop();
    if(UserModeControl::getInstance().interactionMode == EInteractionMode::Synchronization){
      if(syncUpdateResolution < pdTICKS_TO_MS(xTaskGetTickCount() - tick)){
        tick = xTaskGetTickCount();
        NeoPixelMsgData* dataN = new NeoPixelMsgData();
        dataN->list = LightEffect();
        dataN->events = NeoPixelMQEvents::UPDATE_SYNC;
        dataN->mode = ELightMode::None;
        auto absData = std::abs(audioControl.getLastGatin());
        Serial.println(absData);
        if(absData <= 1000){
          dataN->sync = 0;
        }else if(absData > 25000){
          dataN->sync = 255;
        }else{
          dataN->sync = map(std::abs(audioControl.getLastGatin()),1000,32766,0,255);
        }
        neoPixelMsgQueue.send(dataN);
      }
    }
  }
  vTaskDelete(NULL);
}

void shuffleTast(void* parm){
  bool i = true;
  while (1)
  {
    Serial.println("suffle test");
    if(UserModeControl::getInstance().interactionMode != EInteractionMode::Shuffle)
      break;

      AudioMsgData* dataA = new AudioMsgData();
      dataA->list = Playlist();
      dataA->events = AudioMQEvents::UPDATE_ENABLE; 

      NeoPixelMsgData* dataN = new NeoPixelMsgData();
      dataN->list = LightEffect();
      dataN->events = NeoPixelMQEvents::UPDATE_ENABLE;
      dataN->mode = ELightMode::None;

      if(i){
        i = false;
        dataA->enable = false;
        dataN->enable = true;
      }else{
        i = true;
        dataA->enable = true;
        dataN->enable = false;
      }

      audioMsgQueue.send(dataA);
      neoPixelMsgQueue.send(dataN);

      Util::taskDelay(SUFFLE_TIMEOUT);
  }
  
  vTaskDelete(NULL);
}

WebSocketClient wsClient;
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

    //todo serial 
    DynamicJsonDocument doc(512);
    JsonObject json = doc.to<JsonObject>();
    json["event"] = "registerDeviceSession";
    json["name"] = SDUtil::readFile(SDUtil::serialPath_);
    Serial.println(String(json["name"]));
    json["type"] = "Mirror";

    String strJson;
    serializeJson(json, strJson);

    wsClient.sendText(strJson);

  });
  wsClient.onDisconnected([&](uint8_t *payload, size_t length){
    Serial.println("websocket disconnected");
  });
  wsClient.onTextMessageReceived([&](uint8_t *payload, size_t length){
      Serial.println(String(payload,length));
      
      DynamicJsonDocument doc(length * 2);
      Serial.print(deserializeJson(doc, payload,length).c_str());

      if (doc.containsKey("authenticationToken")) {
        SDUtil::authenticationToken_ = String(doc["authenticationToken"]);

        processPlayList(doc["playlist"]);
        processLightEffects(doc["lightEffects"]);
        processUserMode(doc["userMode"]);

      }else if (doc["event"] == "SendLightEffect") {

        processLightEffects(doc["data"]);

      }else if (doc["event"] == "SendSound") {
        JsonObject data = doc["data"];

        String protocol = ssl ? "https://" : "http://";
        long userId = data["userId"];
        int id = data["id"];
        String filename = data["filename"];
        String url = protocol + host + ":" + port + "/api/sound/file/";

        SDUtil::downloadFile(url, id, filename);
      }else if (doc["event"] == "SendPlaylist") {

        processPlayList(doc["data"]);

      }else if(doc["event"] == "SendUserMode") {
        processUserMode(doc["data"]);
      }else if(doc["event"] == "sendHumanDetection") {
        JsonObject data = doc["data"];

        if(UserModeControl::getInstance().operationMode != EOperationMode::Default){
          if(UserModeControl::getInstance().operationMode == EOperationMode::HumanDetectionB)
            UserModeControl::getInstance().humanDetection = !data["isDetected"];
          else
            UserModeControl::getInstance().humanDetection = data["isDetected"];


            AudioMsgData* dataA = new AudioMsgData();
            dataA->list = Playlist();
            dataA->events = AudioMQEvents::UPDATE_ENABLE;
            dataA->enable = false;

            NeoPixelMsgData* dataN = new NeoPixelMsgData();
            dataN->list = LightEffect();
            dataN->events = NeoPixelMQEvents::UPDATE_ENABLE;
            dataN->mode = ELightMode::None;
            dataN->enable = false;


          if(UserModeControl::getInstance().humanDetection){

            switch (UserModeControl::getInstance().interactionMode)
            {
            case EInteractionMode::LightOnly :
              dataN->enable = true;
              dataA->enable = false;
              break;
            case EInteractionMode::SoundOnly :
              dataN->enable = false;
              dataA->enable = true;
              break;
            case EInteractionMode::Shuffle :
              xTaskCreate(shuffleTast,"shuffleTast",10000,NULL,0,NULL);
              break;
            case EInteractionMode::Synchronization :
              dataN->enable = false;
              dataA->enable = true;
              break;
            default:
              break;
            }
          }
          audioMsgQueue.send(dataA);  
          neoPixelMsgQueue.send(dataN);
        }
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

}void processUserMode(JsonObject data){

    AudioMsgData* dataA = new AudioMsgData();
    dataA->list = Playlist();
    dataA->events = AudioMQEvents::UPDATE_ENABLE;
    dataA->enable = false;

    NeoPixelMsgData* dataN = new NeoPixelMsgData();
    dataN->list = LightEffect();
    dataN->events = NeoPixelMQEvents::UPDATE_ENABLE;
    dataN->mode = ELightMode::None;
    dataN->enable = false;

    UserModeControl::getInstance().interactionMode = Util::stringToEInteractionMode(data["interactionMode"]);
    switch (UserModeControl::getInstance().interactionMode)
    {
    case EInteractionMode::LightOnly :
      dataN->enable = true;
      dataA->enable = false;
      break;
    case EInteractionMode::SoundOnly :
      dataN->enable = false;
      dataA->enable = true;
      break;
    case EInteractionMode::Shuffle :
      xTaskCreate(shuffleTast,"shuffleTast",10000,NULL,0,NULL);
      break;
    case EInteractionMode::Synchronization :
      dataN->enable = false;
      dataA->enable = true;
      break;
    default:
      break;
    }
    audioMsgQueue.send(dataA);
    neoPixelMsgQueue.send(dataN);

    UserModeControl::getInstance().operationMode = Util::stringToEOperationMode(data["operationMode"]);

    NeoPixelMsgData* dataN2 = new NeoPixelMsgData();
    dataN2->list = LightEffect();
    dataN2->events = NeoPixelMQEvents::UPDATE_MODE;
    dataN2->mode = Util::stringToELightMode(data["lightMode"]);

    neoPixelMsgQueue.send(dataN2);

}
void processPlayList(JsonObject data){
  
    AudioMsgData* dataA = new AudioMsgData();
    dataA->list = WebSocketClient::parsePlayList(data);
    dataA->events = AudioMQEvents::UPDATE_PLAYLIST;

    audioMsgQueue.send(dataA);
}
void processLightEffects(JsonArray array){
    for(int i = 0; i < array.size(); i++){

    NeoPixelMsgData* dataN = new NeoPixelMsgData();
    dataN->list = WebSocketClient::parseLightEffect(array[i]);
    dataN->events = NeoPixelMQEvents::UPDATE_EFFECT;
    dataN->mode = ELightMode::None;

    neoPixelMsgQueue.send(dataN);

  }
}

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
    audioControl.playNext();
}