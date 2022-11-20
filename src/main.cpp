#include <Arduino.h>

#include <WebServer.h>
#include <ArduinoJson.h>

#include "MdnsModule.h"

#include <string.h>

#include <Adafruit_NeoPixel.h>
#include "Audio.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define I2S_DOUT      2
#define I2S_BCLK      0
#define I2S_LRC       4

WebServer webServer(80);

Adafruit_NeoPixel pixels(10, 32, NEO_GRBW + NEO_KHZ800);

Audio audio;


inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string getEncryptionStr(wifi_auth_mode_t encryptionType){
    switch(encryptionType){
        case WIFI_AUTH_OPEN:
            return "WIFI_AUTH_OPEN";
        case WIFI_AUTH_WEP:
            return "WIFI_AUTH_WEP";
        case WIFI_AUTH_WPA_PSK:
            return "WIFI_AUTH_WPA_PSK";
        case WIFI_AUTH_WPA2_PSK:
            return "WIFI_AUTH_WPA2_PSK";
        case WIFI_AUTH_WPA_WPA2_PSK:
            return "WIFI_AUTH_WPA_WPA2_PSK";
        case WIFI_AUTH_WPA2_ENTERPRISE:
            return "WIFI_AUTH_WPA2_ENTERPRISE";
        case WIFI_AUTH_WPA3_PSK:
            return "WIFI_AUTH_WPA3_PSK";
        case WIFI_AUTH_WPA2_WPA3_PSK:
            return "WIFI_AUTH_WPA2_WPA3_PSK";
        case WIFI_AUTH_WAPI_PSK:
            return "WIFI_AUTH_WAPI_PSK";
        case WIFI_AUTH_MAX:
            return "WIFI_AUTH_MAX";
    }
    return "";
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
void setup() {
    Serial.begin(115200);
    
    if (!SD.begin(5)) {
        Serial.println("# ERROR: can not mount SPIFFS");
        while (1) ;
    }
    
    listDir(SD, "/", 0);


    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // 0...21

//    audio.connecttoFS(SD, "test.mp3");
    audio.connecttoFS(SD, "/test.mp3");

/*
    // Wi0fiModule::getInstance().setIp("192.168.1.1", "192.168.1.1", "255.255.255.0");
    // WifiModule::getInstance().setApInfo("MIRROR-0001-Access-Point", "123456789");
    // WifiModule::getInstance().start();
    
    // MdnsModule::getInstance().mDnsInit("Hello world");

    // webServer.on("/getApList",HTTP_GET, [](){
    //     auto list = WifiModule::getInstance().getApList();
    //     std::string buff;
    //     for(int i =0; i < list.size();i++){
    //         buff.append(list[i].ssid.c_str());
    //         buff.append(" ");
    //         buff.append(list[i].bssid.c_str());
    //         buff.append(" ");
    //         buff.append(getEncryptionStr(list[i].encryptionType));
    //         buff.append("\r\n");
    //     }
    //     webServer.send(200, "text/plain",buff.c_str());
    //     Serial.println(buff.c_str());
    // });
    // webServer.on("/connect",HTTP_POST, [](){
    //     if(webServer.hasArg("plain") == false){
    //         webServer.send(400,"text/plain","no body");
    //         return;
    //     }
    //     DynamicJsonDocument doc(1024);
    //     deserializeJson(doc, webServer.arg("plain"));

    //     if(doc.containsKey("ssid") && doc.containsKey("passwd")){
    //         WifiModule::getInstance().connectWifi(doc["ssid"],doc["passwd"]);
    //         webServer.send(200, "text/plain","connect");

    //     }else{
    //         webServer.send(400,"text/plain","wrong json data");
    //     }

    //     return;

    // });
    // webServer.on("/disconnect",HTTP_POST, [](){
    //     WifiModule::getInstance().disconnectWifi();
    //     webServer.send(200, "text/plain","disconnect");
    //     Serial.println("disconnect");
    // });
    // webServer.on("/play",HTTP_POST, [](){
    
    //     if(webServer.hasArg("plain") == false){
    //         webServer.send(400,"text/plain","no body");
    //         return;
    //     }
    //     DynamicJsonDocument doc(1024);
    //     deserializeJson(doc, webServer.arg("plain"));

    //     if(doc.containsKey("url")){
            
    //         if(!WifiModule::getInstance().isConnectedST()){
    //             webServer.send(400, "text/plain","wifi not connected");
    //             return;
    //         }
            
    //         audio.connecttohost(doc["url"]);

    //         webServer.send(200, "text/plain","asdasd");

    //     }else{
    //         webServer.send(400,"text/plain","wrong json data");
    //     }
    //     return;
    // });
    // webServer.on("/stop",HTTP_GET, [](){

    //     audio.stopSong(); 
    //     webServer.send(200, "text/plain","asdasd");
    // });
    
    // webServer.begin();

    // set audio setting
    */
}
void loop() {
//   webServer.handleClient();
    audio.loop();

}

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     "); Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     "); Serial.println(info);
}
