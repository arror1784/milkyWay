#include <Arduino.h>

#include "WiFi.h"

#include "nvs_flash.h"
#include "Audio.h"
#include "WifiModule.h"
#include "WebServer.h"
#include "MdnsModule.h"

#include <string.h>

#define I2S_DOUT      27
#define I2S_BCLK      26
#define I2S_LRC       25

Audio audio;

WebServer webServer(80);

String ssid =    "301_main_2.4";
String password = "hongsamcoffee3*";

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

void setup() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    Serial.begin(115200);
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    WifiModule::getInstance().setIp("192.168.1.1", "192.168.1.1", "255.255.255.0");
    WifiModule::getInstance().setApInfo("MIRROR-0001-Access-Point", "123456789");
    WifiModule::getInstance().start();  

    MdnsModule::getInstance().mDnsInit("Hello world");

    webServer.on("/getApList",HTTP_GET, [](){
        auto list = WifiModule::getInstance().getApList();
        std::string buff;
        for(int i =0; i < list.size();i++){
            buff.append(list[i].ssid.c_str());
            buff.append(" ");
            buff.append(list[i].bssid.c_str());
            buff.append(" ");
            buff.append(getEncryptionStr(list[i].encryptionType));
            buff.append("\r\n");
        }
        webServer.send(200, "text/plain",buff.c_str());
        Serial.println(buff.c_str());
    });
    
    webServer.begin();


    // while (WiFi.status() != WL_CONNECTED) delay(1500);

    // audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    // audio.setVolume(12); // 0...21

    // audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3

}

void loop() {
  webServer.handleClient();
}


// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}