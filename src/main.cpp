#include <WebServer.h>
#include <ArduinoJson.h>
#include <FFat.h>
#include <sstream>

#include "WebSocketClient.h"
#include "WifiModule.h"
#include "SDUtil.h"
#include "AudioControl.h"
#include "NeoPixel.h"
#include "EepromControl.h"

#include "AudioDownloadMsgQueue.h"
#include "ShuffleTask.h"
#include "NeoPixelTask.h"
#include "AudioTask.h"

static const String host = "192.168.0.195";
static const int port = 6001;
static const bool ssl = false;

//static const String host = "kira-api.wimcorp.dev";
//static const int port = 443;
//static const bool ssl = true;

AudioDownloadMsgQueue audioDownloadMsgQueue(5);

WebSocketClient wsClient;
WebServer webServer(80);

void processUserMode(const JsonObject &data) {
    auto *dataN = new NeoPixelMsgData();
    dataN->lightEffect = LightEffect();
    dataN->events = ENeoPixelMQEvent::UPDATE_MODE;
    dataN->mode = Util::stringToELightMode(data["lightMode"]);

    NeoPixelTask::getInstance().sendMsg(dataN);

    UserModeControl::getInstance().interactionMode = Util::stringToEInteractionMode(data["interactionMode"]);
    UserModeControl::getInstance().operationMode = Util::stringToEOperationMode(data["operationMode"]);

    if (UserModeControl::getInstance().interactionMode == EInteractionMode::Shuffle) {
        auto *dataS = new ShuffleMsgData();
        dataS->events = EShuffleSMQEvent::UPDATE_ENABLE;
        dataS->enable = true;

        ShuffleTask::getInstance().sendMsg(dataS);
    }
    else {
        auto *dataA = new AudioMsgData();
        dataA->list = Playlist();
        dataA->events = EAudioMQEvent::UPDATE_ENABLE;
        dataA->enable = false;

        auto *dataN2 = new NeoPixelMsgData();
        dataN2->lightEffect = LightEffect();
        dataN2->events = ENeoPixelMQEvent::UPDATE_ENABLE;
        dataN2->mode = ELightMode::None;
        dataN2->enable = false;

        if (UserModeControl::getInstance().interactionMode == EInteractionMode::LightOnly) {
            dataN2->enable = true;
            dataA->enable = false;
        }
        if (UserModeControl::getInstance().interactionMode == EInteractionMode::SoundOnly ||
            UserModeControl::getInstance().interactionMode == EInteractionMode::Synchronization) {
            dataN2->enable = false;
            dataA->enable = true;
        }
        AudioTask::getInstance().sendMsg(dataA);
        NeoPixelTask::getInstance().sendMsg(dataN2);
    }
}

void processPlayList(const JsonObject &data) {
    auto dataA = new AudioMsgData();
    dataA->list = WebSocketClient::parsePlayList(data);
    dataA->events = EAudioMQEvent::UPDATE_PLAYLIST;

    AudioTask::getInstance().sendMsg(dataA);
}

void processLightEffects(const JsonArray &array) {
    bool isBlinkingExists = false;
    bool isBreathingExists = false;
    bool isColorChangeExists = false;

    for (auto jsonLightEffect: array) {
        auto *dataN = new NeoPixelMsgData();

        dataN->lightEffect = WebSocketClient::parseLightEffect(jsonLightEffect);
        dataN->events = ENeoPixelMQEvent::UPDATE_EFFECT;
        dataN->mode = ELightMode::None;

        NeoPixelTask::getInstance().sendMsg(dataN);

        if (dataN->mode == ELightMode::Blinking) isBlinkingExists = true;
        else if (dataN->mode == ELightMode::Breathing) isBreathingExists = true;
        else if (dataN->mode == ELightMode::ColorChange) isColorChangeExists = true;
    }

    if (isBlinkingExists) {
        auto *dataN = new NeoPixelMsgData();

        dataN->lightEffect.mode = ELightMode::Blinking;
        dataN->events = ENeoPixelMQEvent::UPDATE_EFFECT;
        dataN->mode = ELightMode::None;

        NeoPixelTask::getInstance().sendMsg(dataN);
    }
    if (isBreathingExists) {
        auto *dataN = new NeoPixelMsgData();

        dataN->lightEffect.mode = ELightMode::Breathing;
        dataN->events = ENeoPixelMQEvent::UPDATE_EFFECT;
        dataN->mode = ELightMode::None;

        NeoPixelTask::getInstance().sendMsg(dataN);
    }
    if (isColorChangeExists) {
        auto *dataN = new NeoPixelMsgData();

        dataN->lightEffect.mode = ELightMode::ColorChange;
        dataN->events = ENeoPixelMQEvent::UPDATE_EFFECT;
        dataN->mode = ELightMode::None;

        NeoPixelTask::getInstance().sendMsg(dataN);
    }
}

void processHumanDetection(const JsonObject &data) {
    if (UserModeControl::getInstance().operationMode != EOperationMode::Default) {
        if (UserModeControl::getInstance().operationMode == EOperationMode::HumanDetectionB) {
            UserModeControl::getInstance().humanDetection = !data["isDetected"];
        }
        else {
            UserModeControl::getInstance().humanDetection = data["isDetected"];
        }

        auto *dataA = new AudioMsgData();
        dataA->list = Playlist();
        dataA->events = EAudioMQEvent::UPDATE_ENABLE;
        dataA->enable = false;

        auto *dataN = new NeoPixelMsgData();
        dataN->lightEffect = LightEffect();
        dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;
        dataN->mode = ELightMode::None;
        dataN->enable = false;

        auto *dataS = new ShuffleMsgData();
        dataS->events = EShuffleSMQEvent::UPDATE_ENABLE;
        dataS->enable = false;

        if (UserModeControl::getInstance().humanDetection) {
            switch (UserModeControl::getInstance().interactionMode) {
                case EInteractionMode::LightOnly :
                    dataN->enable = true;
                    dataA->enable = false;
                    break;
                case EInteractionMode::SoundOnly :
                    dataN->enable = false;
                    dataA->enable = true;
                    break;
                case EInteractionMode::Shuffle :
                    dataS->enable = true;
                    break;
                case EInteractionMode::Synchronization :
                    dataN->enable = false;
                    dataA->enable = true;
                    break;
                default:
                    break;
            }
        }

        AudioTask::getInstance().sendMsg(dataA);
        NeoPixelTask::getInstance().sendMsg(dataN);
        ShuffleTask::getInstance().sendMsg(dataS);
    }
}

void processSendSound(const JsonObject &data) {
    auto *audioDownloadMsg = new AudioDownloadMsgData();

    audioDownloadMsg->id = data["id"];
    audioDownloadMsg->filename = String(data["filename"]);

    audioDownloadMsgQueue.send(audioDownloadMsg);
}

void processPing() {
    DynamicJsonDocument json(512);
    json["event"] = "Pong";

    String strJson;
    serializeJson(json, strJson);

    wsClient.sendText(strJson);
}

bool connectWifi() {
    auto ssid = EepromControl::getInstance().getWifiSsid();
    auto psk = EepromControl::getInstance().getWifiPsk();

    Serial.print("ssid : ");
    Serial.println(ssid);
    Serial.print("password : ");
    Serial.println(psk);

    if (ssid.length() != 0) {
        String status = WifiModule::getInstance().connectWifi(ssid, psk);

        if (status != "WL_CONNECTED") {
            Serial.println("wifi connect fail");
            WifiModule::getInstance().start();
            return false;
        }

        Serial.println("wifi connect");
        WifiModule::getInstance().stop();
        wsClient.connect();
        return true;
    }
    else {
        WifiModule::getInstance().start();
        return false;
    }
}

void receiveWifi() {
    if (!webServer.hasArg("plain")) {
        webServer.send(400, "text/plain", "no plainBody");
        return;
    }
    String plainBody = webServer.arg("plain");
    DynamicJsonDocument doc(plainBody.length() * 2);
    deserializeJson(doc, plainBody);

    if (doc.containsKey("ssid") && doc.containsKey("password")) {
        String strPayload;

        serializeJson(doc, strPayload);

        EepromControl::getInstance().setWifiPsk(doc["ssid"], doc["password"]);

        Serial.println(EepromControl::getInstance().getWifiSsid());
        Serial.println(EepromControl::getInstance().getWifiPsk());

        bool status = connectWifi();

        if (!status) {
            webServer.send(400, "text/plain", "network connect fail");
            return;
        }
        webServer.send(200);
    }
    else {
        webServer.send(400, "text/plain", "wrong json data");
    }
}

void receiveSerial() {
    if (!webServer.hasArg("plain")) {
        webServer.send(400, "text/plain", "no plainBody");
        return;
    }
    String plainBody = webServer.arg("plain");
    DynamicJsonDocument doc(plainBody.length() * 2);
    deserializeJson(doc, plainBody);

    if (doc.containsKey("serial")) {
        String strPayload;

        serializeJson(doc, strPayload);

        EepromControl::getInstance().setSerial(doc["serial"]);
        Serial.println("get serial : " + EepromControl::getInstance().getSerial());

        webServer.send(200);
    }
    else {
        webServer.send(400, "text/plain", "wrong json data");
    }
}

void setup() {
    Serial.begin(115200);
    EepromControl::getInstance().init();

    SDUtil::getInstance().init();
    WiFiClass::mode(WIFI_MODE_STA);
    Serial.println("SERIAL : " + EepromControl::getInstance().getSerial());
    Serial.println("SSID : " + EepromControl::getInstance().getWifiSsid());
    Serial.println("PSK : " + EepromControl::getInstance().getWifiPsk());

    WifiModule::getInstance().setIp("192.168.0.1", "192.168.0.1", "255.255.255.0");
    WifiModule::getInstance().setApInfo(EepromControl::getInstance().getSerial());

    webServer.on("/wifi", HTTP_POST, &receiveWifi);
    webServer.on("/serial", HTTP_POST, &receiveSerial);
    webServer.begin();

    wsClient.setHost(host);
    wsClient.setPort(port);
    wsClient.setWithSsl(ssl);
    wsClient.onConnected([&](uint8_t *payload, size_t length) {
        Serial.println("websocket connected");

        DynamicJsonDocument doc(512);
        JsonObject json = doc.to<JsonObject>();
        json["event"] = "registerDeviceSession";
        json["name"] = EepromControl::getInstance().getSerial();
        Serial.println(String(json["name"]));
        json["type"] = "Mirror";

        String strJson;
        serializeJson(json, strJson);

        wsClient.sendText(strJson);
    });
    wsClient.onDisconnected([&](uint8_t *payload, size_t length) {
        Serial.println("websocket disconnected");
    });
    wsClient.onTextMessageReceived([&](uint8_t *payload, size_t length) {
        DynamicJsonDocument doc(length * 2);
        deserializeJson(doc, payload, length);

        if (doc["event"] != "Ping") {
            String strJson;
            serializeJson(doc, strJson);

            Serial.println(strJson);
        }

        if (doc.containsKey("authenticationToken")) {
            SDUtil::authenticationToken_ = String(doc["authenticationToken"]);
        }
        else if (doc["event"] == "SendLightEffect") {
            processLightEffects(doc["data"]);
        }
        else if (doc["event"] == "SendSound") {
            processSendSound(doc["data"]);
        }
        else if (doc["event"] == "SendPlaylist") {
            processPlayList(doc["data"]);
        }
        else if (doc["event"] == "SendUserMode") {
            processUserMode(doc["data"]);
        }
        else if (doc["event"] == "SendHumanDetection") {
            processHumanDetection(doc["data"]);
        }
        else if (doc["event"] == "Ping") {
            processPing();
        }
    });
    wsClient.onErrorReceived([&](uint8_t *payload, size_t length) {
        Serial.println(String(payload, length));
        Serial.println("websocket errorReceived");
    });

    xTaskCreatePinnedToCore([](void *param) {
        while (1) { NeoPixelTask::getInstance().task(); }
        vTaskDelete(nullptr);
    }, "neoPixelTask", 5000, nullptr, 0, nullptr, 0);
    xTaskCreatePinnedToCore([](void *param) {
        while (1) { ShuffleTask::getInstance().task(); }
        vTaskDelete(nullptr);
    }, "shuffleTask", 5000, nullptr, 0, nullptr, 0);
    xTaskCreatePinnedToCore([](void *param) {
        while (1) { AudioTask::getInstance().task(); }
        vTaskDelete(nullptr);
    }, "audioTask", 10000, nullptr, 1, nullptr, 1);
    connectWifi();
}

void loop() {
    webServer.handleClient();
    wsClient.loop();

    if (WifiModule::getInstance().isConnectedST()) {
        AudioDownloadMsgData *msg = audioDownloadMsgQueue.recv();
        if (msg != nullptr) {
            AudioTask::getInstance().setIsDownloading(true);

            String protocol = ssl ? "https://" : "http://";
            int id = msg->id;
            String filename = msg->filename;
            String url = protocol + host + ":" + port + "/api/sound/file/";

            bool status = SDUtil::downloadFile(url, id, filename);

            if (!status) {
                audioDownloadMsgQueue.send(msg);
            }
            else {
                AudioTask::getInstance().setIsDownloading(false);

                delete msg;
            }
        }
    }
    else {
        connectWifi();
    }
}