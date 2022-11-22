#include "Websocket.h"
#include "WifiModule.h"

WebServer webServer(80);

void setup() {
    WiFi.mode(WIFI_MODE_APSTA);
    Serial.begin(115200);

    WifiModule::getInstance().setIp("192.168.1.1", "192.168.1.1", "255.255.255.0");
    WifiModule::getInstance().setApInfo("MIRROR-0001-Access-Point", "123456789");
    WifiModule::getInstance().start();

    webServer.on("/connect", HTTP_POST, [](){
        if(webServer.hasArg("plain") == false){
            webServer.send(400,"text/plain","no body");
            return;
        }
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, webServer.arg("plain"));

        if(doc.containsKey("ssid") && doc.containsKey("password")){
            bool isConnected = WifiModule::getInstance().connectWifi(doc["ssid"], doc["password"]);
            Serial.println(isConnected ? "Connected!" : "Connect Faild!");
            webServer.send(200, "text/plain", isConnected ? "Connected!" : "Connect Faild!");

        }else{
            webServer.send(400,"text/plain","wrong json data");
        }
        return;

    });
    webServer.begin();
}

void loop() {
    webServer.handleClient();
}