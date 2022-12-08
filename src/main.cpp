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
#include "ShuffleTask.h"
#include "NeoPixelTask.h"
#include "AudioTask.h"

//static const String host = "192.168.0.195";
//static const int port = 6001;
//static const bool ssl = false;

static const String host = "kira-api.wimcorp.dev";
static const int port = 443;
static const bool ssl = true;

AudioFileMsgQueue audioFileMsgQueue(30);

WebSocketClient wsClient;
WebServer webServer(80);

bool isConnectToWifiWithAPI = false;

void updateWithoutShuffle() {
    Serial.println("updateWithoutShuffle");
    // 싱크일 때
    // 감지됨 : 상크 모드 전송, 싱크 활성화, 네오픽셀 활성화, 오디오 활성화
    // 감지안됨 : 싱크 모드 전송, 싱크 홠겅화, 네오픽셀 비활성화, 오디오 비활성화
    if (UserModeControl::getInstance().interactionMode == EInteractionMode::Synchronization) {
        auto *dataN = new NeoPixelMsgData();
        dataN->lightEffect = LightEffect();
        dataN->events = ENeoPixelMQEvent::UPDATE_SYNC;
        dataN->enable = UserModeControl::getInstance().humanDetection;

        NeoPixelTask::getInstance().sendSyncMsg(dataN);
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
            case EInteractionMode::Synchronization :
                dataN->enable = true;
                dataA->enable = true;
                break;
            default:
                break;
        }
    }

    AudioTask::getInstance().sendMsg(dataA);
    NeoPixelTask::getInstance().sendMsg(dataN);
}

void updateShuffle(bool isLightModeChanged) {
    Serial.println("updateShuffle");
    // 셔플일 때
    // 감지됨 : 셔플 모드 전송, 셔플 활성화, 네오픽셀 비활성화
    // 감지안됨 : 셔플 모드 전송, 셔플 비활성화, 네오픽셀 비활성화
    if(isLightModeChanged) {
        auto *dataA = new AudioMsgData();
        dataA->list = Playlist();
        dataA->events = EAudioMQEvent::UPDATE_ENABLE;
        dataA->enable = false;

        auto *dataN = new NeoPixelMsgData();
        dataN->lightEffect = LightEffect();
        dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;
        dataN->mode = ELightMode::None;
        dataN->enable = false;

        AudioTask::getInstance().sendMsg(dataA);
        NeoPixelTask::getInstance().sendMsg(dataN);

        auto *dataS = new ShuffleMsgData();
        dataS->events = EShuffleSMQEvent::UPDATE_ENABLE;
        dataS->enable = UserModeControl::getInstance().humanDetection;

        ShuffleTask::getInstance().sendMsg(dataS);
    }
}

void processHumanDetection(const JsonObject &data) {
    if (UserModeControl::getInstance().operationMode == EOperationMode::Default) {
        UserModeControl::getInstance().humanDetection = true;
    }
    if (UserModeControl::getInstance().operationMode == EOperationMode::HumanDetectionB) {
        UserModeControl::getInstance().humanDetection = !data["isDetected"];
    }
    else {
        UserModeControl::getInstance().humanDetection = data["isDetected"];
    }

    if (UserModeControl::getInstance().interactionMode == EInteractionMode::Shuffle) {
        updateShuffle(true);
    }
    else {
        updateWithoutShuffle();
    }
}

void processUserMode(const JsonObject &data) {
    ELightMode oldLightMode = NeoPixelTask::getInstance().getCurrentMode();
    ELightMode newLightMode = Util::stringToELightMode(data["lightMode"]);

    auto *dataN = new NeoPixelMsgData();
    dataN->lightEffect = LightEffect();
    dataN->events = ENeoPixelMQEvent::UPDATE_MODE;
    dataN->mode = newLightMode;

    NeoPixelTask::getInstance().sendMsg(dataN);

    auto *dataA = new AudioMsgData();
    dataA->list = Playlist();
    dataA->events = EAudioMQEvent::UPDATE_VOLUME;
    dataA->isShuffle = data["soundShuffle"];
    dataA->volume = data["volume"];

    AudioTask::getInstance().sendMsg(dataA);

    EOperationMode oldOperationMode = UserModeControl::getInstance().operationMode;
    EOperationMode newOperationMode = Util::stringToEOperationMode(data["operationMode"]);
    UserModeControl::getInstance().operationMode = newOperationMode;

    if (oldOperationMode == EOperationMode::Default
        && oldOperationMode != newOperationMode) {
        UserModeControl::getInstance().humanDetection = false;
    }
    else if (oldOperationMode == EOperationMode::HumanDetectionB
             && newOperationMode == EOperationMode::HumanDetectionA) {
        UserModeControl::getInstance().humanDetection = !UserModeControl::getInstance().humanDetection;
    }
    else if (oldOperationMode == EOperationMode::HumanDetectionA
             && newOperationMode == EOperationMode::HumanDetectionB) {
        UserModeControl::getInstance().humanDetection = !UserModeControl::getInstance().humanDetection;
    }
    else if (newOperationMode == EOperationMode::Default) {
        UserModeControl::getInstance().humanDetection = true;
    }

    UserModeControl::getInstance().operationMode = newOperationMode;

    EInteractionMode oldInteractionMode = UserModeControl::getInstance().interactionMode;
    EInteractionMode newInteractionMode = Util::stringToEInteractionMode(data["interactionMode"]);
    UserModeControl::getInstance().interactionMode = newInteractionMode;

    if (newInteractionMode == EInteractionMode::Shuffle) {
        updateShuffle(oldLightMode != newLightMode);
        return;
    }
    updateWithoutShuffle();
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

        Serial.print("ssid : ");
        Serial.println(ssid);
        Serial.print("password : ");
        Serial.println(psk);

        if (status != "WL_CONNECTED") {
            Serial.println("wifi connect fail");
            return false;
        }

        Serial.println("wifi connect");
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

        Serial.println(EepromControl::getInstance().getWifiSsid());
        Serial.println(EepromControl::getInstance().getWifiPsk());

        bool status = false;

        for (int i = 0; i < 3; i++) {
            if (connectWifi() == true) {
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
    EepromControl::getInstance().setWifiPsk("Wim", "Wim12345!");

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
        json["token"] = SDUtil::authenticationToken_;

        String strJson;
        serializeJson(json, strJson);

        wsClient.sendText(strJson);
    });
    wsClient.onDisconnected([&](uint8_t *payload, size_t length) {
        Serial.println("websocket disconnected");
    });
    wsClient.onTextMessageReceived([&](uint8_t *payload, size_t length) {
        Serial.println(String(payload, length));
        DynamicJsonDocument doc(length * 2);
        deserializeJson(doc, payload, length);

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
}

void handleAudioFileMsg(bool status, AudioFileMsgData *msg) {
    if (!status) {
        audioFileMsgQueue.send(msg);
    }
    else {
        AudioTask::getInstance().setIsSDAccessing(false);
        if (UserModeControl::getInstance().interactionMode == EInteractionMode::Shuffle) {
            updateShuffle(true);
        }
        else {
            updateWithoutShuffle();
        }

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
    }
    else {
        if (connectWifi()) {
            delay(1000);
            WifiModule::getInstance().stop();
            wsClient.connect();
        }
        else {
            WifiModule::getInstance().start();
        }
    }
}