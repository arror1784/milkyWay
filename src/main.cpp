#include <WebServer.h>
#include <ArduinoJson.h>
#include <FFat.h>
#include <sstream>

#include "WebSocketClient.h"
#include "WifiModule.h"
#include "SDUtil.h"
#include "AudioControl.h"
#include "NeoPixel.h"

#include "AudioMsgQueue.h"
#include "AudioDownloadMsgQueue.h"
#include "ShuffleMsgQueue.h"
#include "NeoPixelTask.h"

#include "EepromControl.h"

#define LED_PIN 32
#define LED_LENGTH 24

#define I2S_DOUT      2
#define I2S_BCLK      0
#define I2S_LRC       4

static const String host = "192.168.219.101";
static const int port = 6001;
static const bool ssl = false;

//static const String host = "kira-api.wimcorp.dev";
//static const int port = 443;
//static const bool ssl = true;

static const int syncUpdateResolution = 10;

void neoPixelTask(void *params) {
    while (1) {
        NeoPixelTask::getInstance().task();
    }

    vTaskDelete(nullptr);
}

AudioControl audioControl(I2S_LRC, I2S_BCLK, I2S_DOUT);

AudioMsgQueue audioMsgQueue(5);
ShuffleMsgQueue shuffleMsgQueue(5);

void audioTask(void *params) {
    const long shuffleAudioTIme = 5500;

    int volume = 10;
    std::vector<int> gains;
    audioControl.setVolume(volume); // 0...21
    TickType_t tick = xTaskGetTickCount();
    bool isShuffle = false;
    unsigned long nextTick = 0xFFFFFFFF;

    while (1) {
        AudioMsgData *msg = audioMsgQueue.recv();

        if (audioControl.isDownloading()) {
            if (msg != nullptr) delete msg;

            continue;
        }
        if (msg != nullptr) {
            if (msg->events == EAudioMQEvent::UPDATE_PLAYLIST) {
                Serial.println("updatePLAYLIST");
                auto &list = msg->list;
                audioControl.setPlayList(list);
            }
            else if (msg->events == EAudioMQEvent::UPDATE_ENABLE) {
                if (msg->enable) {
                    audioControl.resume();
                    isShuffle = msg->isShuffle;

                    if (isShuffle) {
                        nextTick = millis() + shuffleAudioTIme;
                    }
                }
                else {
                    audioControl.pause();
                }
            }

            delete msg;
        }
        audioControl.loop();

        if (UserModeControl::getInstance().interactionMode == EInteractionMode::Synchronization) {
            if (syncUpdateResolution < pdTICKS_TO_MS(xTaskGetTickCount() - tick)) {
                auto gain = std::abs(audioControl.getLastGain()) / volume;

                tick = xTaskGetTickCount();
                auto *dataN = new NeoPixelMsgData();
                dataN->lightEffect = LightEffect();
                dataN->events = ENeoPixelMQEvent::UPDATE_SYNC;
                dataN->mode = ELightMode::None;

                if (gains.size() > 10) {
                    gains.erase(gains.begin());
                }
                gains.push_back(gain);

                int total = 0;
                for (auto savedGain: gains) {
                    total += savedGain;
                }

                dataN->sync = total / gains.size();

                NeoPixelTask::getInstance().sendMsg(dataN);
            }
        }

        if (isShuffle && nextTick <= millis()) {
            auto *dataS = new ShuffleMsgData();

            dataS->events = EShuffleSMQEvent::FINISH_SOUND;
            dataS->enable = true;

            shuffleMsgQueue.send(dataS);

            nextTick = 0xFFFFFFFF;
        }
    }
    vTaskDelete(nullptr);
}

void shuffleTask(void *params) {
    const long shuffleSleepTIme = 1500;

    unsigned long nextTick = 0xFFFFFFFF;
    bool isNextSound = true;

    while (1) {
        ShuffleMsgData *dataS = shuffleMsgQueue.recv();

        if (dataS != nullptr) {
            if (dataS->events == EShuffleSMQEvent::UPDATE_ENABLE) {
                if (dataS->enable) {
                    auto *dataA = new AudioMsgData();
                    dataA->isShuffle = true;
                    audioMsgQueue.send(dataA);

                    auto *dataN = new NeoPixelMsgData();
                    dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;
                    dataN->enable = false;
                    dataN->isShuffle = true;
                    NeoPixelTask::getInstance().sendMsg(dataN);
                }
            }
            else if (dataS->events == EShuffleSMQEvent::FINISH_NEO_PIXEL) {
                isNextSound = false;
                nextTick = millis() + shuffleSleepTIme;
            }
            else if (dataS->events == EShuffleSMQEvent::FINISH_SOUND) {
                isNextSound = true;
                nextTick = millis() + shuffleSleepTIme;
            }

            delete dataS;
        }
        if (nextTick <= millis()) {
            if (isNextSound) {
                auto *dataA = new AudioMsgData();
                dataA->events = EAudioMQEvent::UPDATE_ENABLE;
                dataA->enable = true;
                dataA->isShuffle = true;
                audioMsgQueue.send(dataA);
            }
            else {
                auto *dataN = new NeoPixelMsgData();
                dataN->events = ENeoPixelMQEvent::UPDATE_ENABLE;
                dataN->enable = true;
                dataN->isShuffle = true;
                NeoPixelTask::getInstance().sendMsg(dataN);
            }
            nextTick = 0xFFFFFFFF;
        }
    }

    vTaskDelete(nullptr);
}

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

        shuffleMsgQueue.send(dataS);
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
        audioMsgQueue.send(dataA);
        NeoPixelTask::getInstance().sendMsg(dataN2);
    }
}

void processPlayList(const JsonObject &data) {
    auto dataA = new AudioMsgData();
    dataA->list = WebSocketClient::parsePlayList(data);
    dataA->events = EAudioMQEvent::UPDATE_PLAYLIST;

    audioMsgQueue.send(dataA);
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

WebSocketClient wsClient;
WebServer webServer(80);

bool connectWifi() {
    auto ssid = EepromControl::getInstance().getWifiSsid();
    auto psk = EepromControl::getInstance().getWifiPsk();

//    Serial.print("ssid : ");
//    Serial.println(ssid);
//    Serial.print("password : ");
//    Serial.println(psk);

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

//        Serial.println(EepromControl::getInstance().getWifiSsid());
//        Serial.println(EepromControl::getInstance().getWifiPsk());

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
//        Serial.println("get serial : " + EepromControl::getInstance().getSerial());

        webServer.send(200);
    }
    else {
        webServer.send(400, "text/plain", "wrong json data");
    }
}

bool isFirstConnection = true;
AudioDownloadMsgQueue audioDownloadMsgQueue(5);

void setup() {
    Serial.begin(115200);
    EepromControl::getInstance().init();

    SDUtil::getInstance().init();
    WiFiClass::mode(WIFI_MODE_STA);
//    Serial.println("SERIAL : " + EepromControl::getInstance().getSerial());
//    Serial.println("SSID : " + EepromControl::getInstance().getWifiSsid());
//    Serial.println("PSK : " + EepromControl::getInstance().getWifiPsk());

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
        json["isFirstConnection"] = isFirstConnection;
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
            JsonObject data = doc["data"];

            auto *audioDownloadMsg = new AudioDownloadMsgData();

            audioDownloadMsg->id = data["id"];
            audioDownloadMsg->filename = String(data["filename"]);

            audioDownloadMsgQueue.send(audioDownloadMsg);
        }
        else if (doc["event"] == "SendPlaylist") {
            processPlayList(doc["data"]);
        }
        else if (doc["event"] == "SendUserMode") {
            processUserMode(doc["data"]);
        }
        else if (doc["event"] == "SendHumanDetection") {
            JsonObject data = doc["data"];

            if (UserModeControl::getInstance().operationMode != EOperationMode::Default) {
                if (UserModeControl::getInstance().operationMode == EOperationMode::HumanDetectionB)
                    UserModeControl::getInstance().humanDetection = !data["isDetected"];
                else
                    UserModeControl::getInstance().humanDetection = data["isDetected"];

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

                audioMsgQueue.send(dataA);
                NeoPixelTask::getInstance().sendMsg(dataN);
                shuffleMsgQueue.send(dataS);
            }
        }
        else if (doc["event"] == "Ping") {
            DynamicJsonDocument json(512);
            json["event"] = "Pong";

            String strJson;
            serializeJson(json, strJson);

            wsClient.sendText(strJson);
        }
    });
    wsClient.onErrorReceived([&](uint8_t *payload, size_t length) {
        Serial.println(String(payload, length));
        Serial.println("websocket errorReceived");
    });

    xTaskCreatePinnedToCore(neoPixelTask, "neoPixelTask", 5000, nullptr, 0, nullptr, 0);
    xTaskCreatePinnedToCore(shuffleTask, "shuffleTask", 5000, nullptr, 0, nullptr, 0);
    xTaskCreatePinnedToCore(audioTask, "audioTask", 10000, nullptr, 1, nullptr, 1);
    connectWifi();
}

void loop() {
    webServer.handleClient();
    wsClient.loop();

    // TODO: 큐를 임베디드 기기가 아닌 서버에서 구현하고, 이를 패치해 오는 방식을 바꾸는 것도 고려

    if (WifiModule::getInstance().isConnectedST()) {
        AudioDownloadMsgData *msg = audioDownloadMsgQueue.recv();
        if (msg != nullptr) {
            audioControl.setIsDownloading(true);

            String protocol = ssl ? "https://" : "http://";
            int id = msg->id;
            String filename = msg->filename;
            String url = protocol + host + ":" + port + "/api/sound/file/";

            bool status = SDUtil::downloadFile(url, id, filename);

            if (!status) {
                audioDownloadMsgQueue.send(msg);
            }
            else {
                audioControl.setIsDownloading(false);

                delete msg;
            }
        }
    }
    else {
        connectWifi();
    }
}

void audio_info(const char *info) {
    Serial.print("info        ");
    Serial.println(info);
}

void audio_eof_mp3(const char *info) {  //end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
    audioControl.playNext();
}