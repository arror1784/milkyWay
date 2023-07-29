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

#include "AudioFileMsgQueue.h"
#include "PingPongTask.h"
#include "NeoPixelTask.h"
#include "AudioTask.h"

//static const String host = "192.168.0.195";
//static const int port = 6001;
//static const bool ssl = false;

static const String host = "13.125.102.123";
static const int port = 11002;
static const bool ssl = false;

//static const String host = "kira-api.wimcorp.dev";
//static const int port = 443;
//static const bool ssl = true;

AudioFileMsgQueue audioFileMsgQueue(30);

WebSocketClient wsClient;
WebServer webServer(80);

bool isConnectToWifiWithAPI = false;

int wifiConnectCount = 0;

void disableAllTask() {
    Serial.println("disableAllTask");

    // 오디오 비활성화
    auto *dataA = new AudioMsgData();
    dataA->list = Playlist();
    dataA->events = EAudioMQEvent::UPDATE_ENABLE;
    dataA->enable = false;

    // 네오픽셀 비활성화
    auto *dataN = new NeoPixelMsgData();
    dataN->lightEffect = LightEffect();
    dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;
    dataN->enable = false;

    // 핑퐁 비활성화
    auto *dataP = new PingPongMsgData();
    dataP->events = EPingPongMQEvent::UPDATE_ENABLE;
    dataP->enable = false;

    AudioTask::getInstance().sendMsg(dataA);
    NeoPixelTask::getInstance().sendMsg(dataN);
    PingPongMsgQueue::getInstance().send(dataP);
}

void handleInteractionMode() {
    Serial.println("handleInteractionMode");
    auto interactionMode = UserModeControl::getInstance().interactionMode;

    auto *dataA = new AudioMsgData();
    dataA->list = Playlist();
    dataA->events = EAudioMQEvent::UPDATE_ENABLE;
    dataA->enable = true;

    auto *dataN = new NeoPixelMsgData();
    dataN->lightEffect = LightEffect();
    dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;
    dataN->mode = ELightMode::None;
    dataN->enable = true;

    auto *dataP = new PingPongMsgData();
    dataP->events = EPingPongMQEvent::UPDATE_ENABLE;
    dataP->enable = true;

    if (EInteractionMode::LightOnly == interactionMode) {
        NeoPixelTask::getInstance().sendMsg(dataN);

        delete dataA;
        delete dataP;
    }
    else if (EInteractionMode::SoundOnly == interactionMode) {
        AudioTask::getInstance().sendMsg(dataA);

        delete dataN;
        delete dataP;
    }
    else if (EInteractionMode::Synchronization == interactionMode) {
        NeoPixelTask::getInstance().sendMsg(dataN);
        AudioTask::getInstance().sendMsg(dataA);

        delete dataP;
    }
    else { // EInteractionMode::Synchronization == EInteractionMode::PingPong
        PingPongMsgQueue::getInstance().send(dataP);

        delete dataN;
        delete dataA;
    }
}

void processHumanDetection(const JsonObject &data) {
//    Serial.println("processHumanDetection");

    bool oldHumanDetection = UserModeControl::getInstance().humanDetection;
    bool newHumanDetection;

    if (UserModeControl::getInstance().operationMode == EOperationMode::Default) {
        newHumanDetection = true;
    }
    else if (UserModeControl::getInstance().operationMode == EOperationMode::HumanDetectionB) {
        newHumanDetection = data["isDetected"];
    }
    else {
        newHumanDetection = !data["isDetected"];
    }

    if (newHumanDetection != oldHumanDetection) {
        disableAllTask();
        if (newHumanDetection) {
            handleInteractionMode();
        }
    }
    UserModeControl::getInstance().humanDetection = newHumanDetection;

//    Serial.println("newHumanDetection : " + String(UserModeControl::getInstance().humanDetection));
}

void processUserMode(const JsonObject &data) {
    Serial.println("processUserMode");

    EOperationMode oldOperationMode = UserModeControl::getInstance().operationMode;
    EOperationMode newOperationMode = Util::stringToEOperationMode(data["operationMode"]);
    UserModeControl::getInstance().operationMode = newOperationMode;

    EInteractionMode oldInteractionMode = UserModeControl::getInstance().interactionMode;
    EInteractionMode newInteractionMode = Util::stringToEInteractionMode(data["interactionMode"]);
    UserModeControl::getInstance().interactionMode = newInteractionMode;

    bool oldHumanDetection = UserModeControl::getInstance().humanDetection;
    bool newHumanDetection = oldHumanDetection;

    /*
     * D -> D = true
     * D -> A = false
     * D -> B = true
     *
     * A -> D = true
     * A -> A = 유지
     * A -> B = 반전
     *
     * B -> D = true
     * B -> A = 반전
     * B -> B = 유지
    **/

    if (newOperationMode == EOperationMode::Default) {
        newHumanDetection = true;
    }
    else if (oldOperationMode == EOperationMode::Default ||
             oldOperationMode == EOperationMode::N) {
        if (newOperationMode == EOperationMode::HumanDetectionA) {
            newHumanDetection = true;
        }
        else {
            newHumanDetection = false;
        }
    }
    else if (oldOperationMode == EOperationMode::HumanDetectionA) {
        if (newOperationMode == EOperationMode::HumanDetectionB) {
            newHumanDetection = !UserModeControl::getInstance().humanDetection;
        }
    }
    else if (oldOperationMode == EOperationMode::HumanDetectionB) {
        if (newOperationMode == EOperationMode::HumanDetectionA) {
            newHumanDetection = !UserModeControl::getInstance().humanDetection;
        }
    }

    UserModeControl::getInstance().humanDetection = newHumanDetection;

    if (oldInteractionMode != newInteractionMode
        || oldHumanDetection != newHumanDetection
        || !newHumanDetection) {
        disableAllTask();
    }

    if (oldInteractionMode == EInteractionMode::Synchronization
        && newInteractionMode != EInteractionMode::Synchronization) {
        auto *dataN = new NeoPixelMsgData();
        dataN->events = ENeoPixelMQEvent::UPDATE_SYNC;
        dataN->enable = false;

        NeoPixelTask::getInstance().sendSyncMsg(dataN);
    }

    auto *dataN = new NeoPixelMsgData();
    dataN->lightEffect = LightEffect();
    dataN->events = ENeoPixelMQEvent::UPDATE_MODE;
    dataN->mode = Util::stringToELightMode(data["lightMode"]);

    NeoPixelTask::getInstance().sendMsg(dataN);

    auto *dataA = new AudioMsgData();
    dataA->list = Playlist();
    dataA->events = EAudioMQEvent::UPDATE_VOLUME_SHUFFLE;
    dataA->isShuffle = data["soundShuffle"];

    AudioTask::getInstance().sendMsg(dataA);

    if (newHumanDetection) {
        if (oldInteractionMode != newInteractionMode || oldHumanDetection != newHumanDetection) {
            handleInteractionMode();
        }
    }
}

void processDeviceConfig(const JsonObject &data) {
    auto *dataA = new AudioMsgData();
    dataA->list = Playlist();
    dataA->events = EAudioMQEvent::UPDATE_VOLUME_SHUFFLE;
    dataA->volume = data["volume"];

    AudioTask::getInstance().sendMsg(dataA);
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

        NeoPixelTask::getInstance().sendMsg(dataN);

        if (dataN->lightEffect.mode == ELightMode::Blinking) isBlinkingExists = true;
        else if (dataN->lightEffect.mode == ELightMode::Breathing) isBreathingExists = true;
        else if (dataN->lightEffect.mode == ELightMode::ColorChange) isColorChangeExists = true;
    }

    if (!isBlinkingExists) {
        auto *dataN = new NeoPixelMsgData();

        dataN->lightEffect.mode = ELightMode::Blinking;
        dataN->events = ENeoPixelMQEvent::UPDATE_EFFECT;

        NeoPixelTask::getInstance().sendMsg(dataN);
    }
    if (!isBreathingExists) {
        auto *dataN = new NeoPixelMsgData();

        dataN->lightEffect.mode = ELightMode::Breathing;
        dataN->events = ENeoPixelMQEvent::UPDATE_EFFECT;

        NeoPixelTask::getInstance().sendMsg(dataN);
    }
    if (!isColorChangeExists) {
        auto *dataN = new NeoPixelMsgData();

        dataN->lightEffect.mode = ELightMode::ColorChange;
        dataN->events = ENeoPixelMQEvent::UPDATE_EFFECT;

        NeoPixelTask::getInstance().sendMsg(dataN);
    }

    auto *dataN = new NeoPixelMsgData();

    dataN->events = ENeoPixelMQEvent::UPDATE_MODE;
    dataN->mode = ELightMode::None;

    NeoPixelTask::getInstance().sendMsg(dataN);
}

void processSendSound(const JsonObject &data) {
    auto *audioFileMsg = new AudioFileMsgData();

    audioFileMsg->event = EAudioFileEvent::DOWNLOAD;
    audioFileMsg->id = data["id"];
    audioFileMsg->filename = String(data["filename"]);

    audioFileMsgQueue.send(audioFileMsg);
}

void processSendUnplayableSounds(const JsonArray &data) {
    for (auto jsonSound: data) {
        auto *audioDownloadMsg = new AudioFileMsgData();

        audioDownloadMsg->id = jsonSound["id"];
        audioDownloadMsg->filename = String(jsonSound["filename"]);

        audioFileMsgQueue.send(audioDownloadMsg);
    }
}

void processSendDeletedSound(const JsonObject &data) {
    auto *audioFileMsg = new AudioFileMsgData();

    audioFileMsg->event = EAudioFileEvent::DELETE;
    audioFileMsg->id = data["id"];
    audioFileMsg->filename = String(data["filename"]);

    if (audioFileMsg->id == AudioTask::getInstance().getCurrentSound().id) {
        auto *dataA = new AudioMsgData();
        dataA->events = EAudioMQEvent::UPDATE_DELETE_CURRENT_SOUND;

        AudioTask::getInstance().sendMsg(dataA);
    }

    audioFileMsgQueue.send(audioFileMsg);
}

void processPing() {
    wsClient.sendPong();
}

bool connectWifi() {
    auto ssid = EepromControl::getInstance().getWifiSsid();
    auto psk = EepromControl::getInstance().getWifiPsk();

    if (ssid.length() != 0) {
        String status = WifiModule::getInstance().connectWifi(ssid, psk);

        Serial.print("connectWifi : ssid : ");
        Serial.println(ssid);
        Serial.print("connectWifi : password : ");
        Serial.println(psk);

        if (status != "WL_CONNECTED") {
            Serial.println("connectWifi : wifi connect fail");
            return false;
        }

        Serial.println("connectWifi : wifi connect");
        return true;
    }
    else {
        return false;
    }
}

void receiveWifi() {
    isConnectToWifiWithAPI = true;
    if (!webServer.hasArg("plain")) {
        webServer.send(400, "text/plain", "no plainBody");
        isConnectToWifiWithAPI = false;
        return;
    }
    String plainBody = webServer.arg("plain");
    DynamicJsonDocument doc(plainBody.length() * 2);
    deserializeJson(doc, plainBody);

    if (doc.containsKey("ssid") && doc.containsKey("password"), doc.containsKey("token")) {
        String strPayload;

        serializeJson(doc, strPayload);

        SDUtil::authenticationToken_ = String(doc["token"]);
        EepromControl::getInstance().setWifiPsk(doc["ssid"], doc["password"]);

        Serial.println("receiveWifi : " + EepromControl::getInstance().getWifiSsid());
        Serial.println("receiveWifi : " + EepromControl::getInstance().getWifiPsk());

        bool status = false;

        for (int i = 0; i < 3; i++) {
            if (connectWifi()) {
                status = true;
                break;
            }
        }

        if (!status) {
            EepromControl::getInstance().setWifiPsk("", "");
            isConnectToWifiWithAPI = false;

            webServer.send(400, "text/plain", "network connect fail");
            return;
        }

        isConnectToWifiWithAPI = false;
        webServer.send(200);
        delay(5000);

        WifiModule::getInstance().stop();

        wsClient.connect();

        wifiConnectCount = 0;
    }
    else {
        isConnectToWifiWithAPI = false;

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
        Serial.println("receiveSerial : serial : " + EepromControl::getInstance().getSerial());

        webServer.send(200);
    }
    else {
        webServer.send(400, "text/plain", "wrong json data");
    }
}

void setup() {
    Serial.begin(115200);
    EepromControl::getInstance().init();
    EepromControl::getInstance().setSerial("Kira_Mirror_00001");
    EepromControl::getInstance().setWifiPsk("", "");

    SDUtil::getInstance().init();
    WiFiClass::mode(WIFI_MODE_STA);
    Serial.println("setup : SERIAL : " + EepromControl::getInstance().getSerial());
    Serial.println("setup : SSID : " + EepromControl::getInstance().getWifiSsid());
    Serial.println("setup : PSK : " + EepromControl::getInstance().getWifiPsk());

    WifiModule::getInstance().setIp("192.168.0.1", "192.168.0.1", "255.255.255.0");
    WifiModule::getInstance().setApInfo(EepromControl::getInstance().getSerial());

    webServer.on("/wifi", HTTP_POST, &receiveWifi);
    webServer.on("/serial", HTTP_POST, &receiveSerial);
    webServer.begin();

    wsClient.setHost(host);
    wsClient.setPort(port);
    wsClient.setWithSsl(ssl);
    wsClient.onConnected([&](uint8_t *payload, size_t length) {
        Serial.println("onConnected : websocket connected");

        DynamicJsonDocument doc(512);
        JsonObject json = doc.to<JsonObject>();
        json["event"] = "registerDeviceSession";
        json["name"] = EepromControl::getInstance().getSerial();
        json["type"] = "Mirror";
        json["ledCount"] = NeoPixelTask::getInstance().getLedCount();
        json["token"] = SDUtil::authenticationToken_;

        String strJson;
        serializeJson(json, strJson);

        wsClient.sendText(strJson);
    });
    wsClient.onDisconnected([&](uint8_t *payload, size_t length) {
        Serial.println("onDisconnected : websocket disconnected");
    });
    wsClient.onTextMessageReceived([&](uint8_t *payload, size_t length) {
        DynamicJsonDocument doc(length * 2);
        deserializeJson(doc, payload, length);

        if (doc["event"] != "SendHumanDetection") {
            String strJson;
            serializeJson(doc, strJson);
            Serial.println("onTextMessageReceived : " + strJson);
        }

        if (doc.containsKey("authenticationToken")) {
            String token = String(doc["authenticationToken"]);

            if (token.isEmpty()) {
                wsClient.disconnect();
                EepromControl::getInstance().setWifiPsk("", "");
                WifiModule::getInstance().start();

                return;
            }

            SDUtil::authenticationToken_ = token;
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
        else if (doc["event"] == "SendConfig") {
            processDeviceConfig(doc["data"]);
        }
        else if (doc["event"] == "SendHumanDetection") {
            processHumanDetection(doc["data"]);
        }
        else if (doc["event"] == "SendUnplayableSounds") {
            processSendUnplayableSounds(doc["data"]);
        }
        else if (doc["event"] == "SendDeletedSound") {
            processSendDeletedSound(doc["data"]);
        }
    });
    wsClient.onPingMessageReceived([&](uint8_t *payload, size_t length) {
        processPing();
    });


//    xTaskCreatePinnedToCore([](void *param) {
//        while (1) { NeoPixelTask::getInstance().task(); }
//        vTaskDelete(nullptr);
//    }, "neoPixelTask", 5000, nullptr, 0, nullptr, 0);
    xTaskCreatePinnedToCore([](void *param) {
        while (1) { PingPongTask::getInstance().task(); }
        vTaskDelete(nullptr);
    }, "pingPongTask", 5000, nullptr, 0, nullptr, 0);
    xTaskCreatePinnedToCore([](void *param) {
        while (1) { AudioTask::getInstance().task(); }
        vTaskDelete(nullptr);
    }, "audioTask", 10000, nullptr, 1, nullptr, 1);
}

void handleAudioFileMsg(bool status, AudioFileMsgData *msg) {
    if (!status) {
        audioFileMsgQueue.send(msg);
    }
    else {
        AudioTask::getInstance().setIsSDAccessing(false);

        delete msg;
    }
}

void loop() {
    webServer.handleClient();
    wsClient.loop();

    if (WifiModule::getInstance().isConnectedST() && !isConnectToWifiWithAPI) {
        AudioFileMsgData *msg = audioFileMsgQueue.recv();
        if (msg != nullptr) {
            AudioTask::getInstance().setIsSDAccessing(true);
            if (msg->event == EAudioFileEvent::DOWNLOAD) {
                String protocol = ssl ? "https://" : "http://";
                int id = msg->id;
                String filename = msg->filename;
                String url = protocol + host + ":" + port + "/api/sound/file/";

                bool status = SDUtil::downloadFile(url, id, filename);

                handleAudioFileMsg(status, msg);
            }
            else if (msg->event == EAudioFileEvent::DELETE) {
                AudioTask::getInstance().setIsSDAccessing(true);
                bool status = SDUtil::deleteFile(msg->filename);

                handleAudioFileMsg(status, msg);
            }
        }

        NeoPixelTask::getInstance().task();
    }
    else {
        if (wifiConnectCount < 5) {
            if (connectWifi()) {
                delay(1000);
                WifiModule::getInstance().stop();
                wsClient.connect();
            }
            else {
                wifiConnectCount += 1;
                Serial.println("loop : wifiConnectCount : " + String(wifiConnectCount));

                if (wifiConnectCount == 5) {
                    WifiModule::getInstance().start();
                }
            }
        }
    }
}