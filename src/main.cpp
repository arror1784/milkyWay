#include <WebServer.h>
#include <ArduinoJson.h>
#include <FFat.h>
#include <sstream>

#include "WebSocketClient.h"
#include "WifiModule.h"
#include "SDUtil.h"
#include "AudioControl.h"
#include "NeoPixel.h"
#include "UserModeControl.h"

#include "AudioMsgQueue.h"
#include "NeoPixelMsgQueue.h"
#include "ShuffleMsgQueue.h"

#define LED_PIN 32
#define LED_LENGTH 24

#define I2S_DOUT      2
#define I2S_BCLK      0
#define I2S_LRC       4

#define SUFFLE_TIMEOUT 5500

static const String host = "192.168.219.108";
static const int port = 6001;
static const bool ssl = false;

//static const String host = "kira-api.wimcorp.dev";
//static const int port = 443;
//static const bool ssl = true;
static const int syncUpdateResolution = 10;

void processUserMode(JsonObject data);

void processPlayList(JsonObject data);

void processLightEffects(JsonArray array);

NeoPixelMsgQueue neoPixelMsgQueue(5);

void neoPixelTask(void *params) {
    Neopixel neopixel(LED_LENGTH, LED_PIN, NEO_GRBW | NEO_KHZ800);

    while (1) {
        NeoPixelMsgData *msg = neoPixelMsgQueue.recv();
        if (msg != nullptr) {
            if (msg->events == NeoPixelMQEvents::UPDATE_EFFECT) {
                neopixel.setLightEffects(msg->list);
            }
            else if (msg->events == NeoPixelMQEvents::UPDATE_MODE) {
                neopixel.changeMode(msg->mode);
            }
            else if (msg->events == NeoPixelMQEvents::UPDATE_ENABLE) {
//                Serial.println(String("led msg->enable : ") + msg->enable);
                neopixel.setEnable(msg->enable);
            }
            else if (msg->events == NeoPixelMQEvents::UPDATE_SYNC) {
                neopixel.sync(msg->sync);
            }

            delete msg;
        }
        neopixel.loop();
    }

    vTaskDelete(nullptr);
}

AudioControl audioControl(I2S_LRC, I2S_BCLK, I2S_DOUT);
AudioMsgQueue audioMsgQueue(5);

int volume = 21;

void audioTask(void *params) {
    audioControl.setVolume(volume); // 0...21
    TickType_t tick = xTaskGetTickCount();

    while (1) {
        AudioMsgData *msg = audioMsgQueue.recv();
        if (msg != nullptr) {

            if (msg->events == AudioMQEvents::UPDATE_PLAYLIST) {
                Serial.println("updatePLAYLIST");
                auto &list = msg->list;
                audioControl.setPlayList(list);
            }
            else if (msg->events == AudioMQEvents::UPDATE_ENABLE) {
//                Serial.println(String("msg->enable : ") + msg->enable);
                if (msg->enable)
                    audioControl.resume();
                else
                    audioControl.pause();
            }

            delete msg;
        }
        audioControl.loop();
        if (UserModeControl::getInstance().interactionMode == EInteractionMode::Synchronization) {
            if (syncUpdateResolution < pdTICKS_TO_MS(xTaskGetTickCount() - tick)) {
                tick = xTaskGetTickCount();
                auto *dataN = new NeoPixelMsgData();
                dataN->list = LightEffect();
                dataN->events = NeoPixelMQEvents::UPDATE_SYNC;
                dataN->mode = ELightMode::None;
                auto absData = std::abs(audioControl.getLastGatin()) / volume;
                dataN->sync = absData;
                neoPixelMsgQueue.send(dataN);
            }
        }
    }
    vTaskDelete(nullptr);
}

ShuffleMsgQueue shuffleMsgQueue(5);

void shuffleTask(void *params) {
    bool i = true;
    bool flag = false;
    while (1) {
        ShuffleMsgData *msg = nullptr;
        ShuffleMsgData *temp = nullptr;
        while (1) {
            temp = shuffleMsgQueue.recv();
            if (temp != nullptr) {
                if (msg != nullptr)
                    delete msg;
                msg = temp;
            }
            else {
                break;
            }
        }
        if (msg != nullptr) {
            flag = msg->enable;
            delete msg;
            msg = nullptr;
        }
        if (flag) {
            auto *dataA = new AudioMsgData();
            dataA->list = Playlist();
            dataA->events = AudioMQEvents::UPDATE_ENABLE;

            auto *dataN = new NeoPixelMsgData();
            dataN->list = LightEffect();
            dataN->events = NeoPixelMQEvents::UPDATE_ENABLE;
            dataN->mode = ELightMode::None;

            if (i) {
                i = false;
                dataA->enable = false;
                dataN->enable = true;
            }
            else {
                i = true;
                dataA->enable = true;
                dataN->enable = false;
            }

            audioMsgQueue.send(dataA);
            neoPixelMsgQueue.send(dataN);

            Util::taskDelay(SUFFLE_TIMEOUT);
        }
        else {
            Util::taskDelay(100);
        }
    }

    vTaskDelete(nullptr);
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
        String strPayload;

        serializeJson(doc, strPayload);

        SDUtil::writeFile(SDUtil::wifiInfoPath_, strPayload);

        String status = WifiModule::getInstance().connectWifi(doc["ssid"], doc["password"]);

        if (status != "WL_CONNECTED") {
            webServer.send(400, "text/plain", "network connect fail");
            return;
        }

        wsClient.connect();
        webServer.send(200);
    }
    else {
        webServer.send(400, "text/plain", "wrong json data");
    }
}

void setup() {
    Serial.begin(115200);

    SDUtil::getInstance().init();
    WiFiClass::mode(WIFI_MODE_STA);
    Serial.println(SDUtil::getInstance().getSerial());
    WifiModule::getInstance().setIp("192.168.0.1", "192.168.0.1", "255.255.255.0");
    WifiModule::getInstance().setApInfo(SDUtil::getInstance().getSerial());
    WifiModule::getInstance().start();

    wsClient.setHost(host);
    wsClient.setPort(port);
    wsClient.setWithSsl(ssl);
    wsClient.onConnected([&](uint8_t *payload, size_t length) {
        Serial.println("websocket connected");

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
    wsClient.onDisconnected([&](uint8_t *payload, size_t length) {
        Serial.println("websocket disconnected");
    });
    wsClient.onTextMessageReceived([&](uint8_t *payload, size_t length) {
        Serial.println(String(payload, length));

        DynamicJsonDocument doc(length * 2);
        Serial.print(deserializeJson(doc, payload, length).c_str());

        if (doc.containsKey("authenticationToken")) {
            SDUtil::authenticationToken_ = String(doc["authenticationToken"]);
        }
        else if (doc["event"] == "SendLightEffect") {

            processLightEffects(doc["data"]);

        }
        else if (doc["event"] == "SendSound") {
//            JsonObject data = doc["data"];
//
//            auto *audioDownloadMsg = new AudioDownloadMsgData();
//
//            audioDownloadMsg->id = data["id"];
//            audioDownloadMsg->filename = String(data["filename"]);
//
//            audioDownloadMsgQueue.send(audioDownloadMsg);
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
                dataA->events = AudioMQEvents::UPDATE_ENABLE;
                dataA->enable = false;

                auto *dataN = new NeoPixelMsgData();
                dataN->list = LightEffect();
                dataN->events = NeoPixelMQEvents::UPDATE_ENABLE;
                dataN->mode = ELightMode::None;
                dataN->enable = false;

                auto *dataS = new ShuffleMsgData();
                dataS->events = ShuffleSMQEvents::UPDATE_ENABLE;
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


                Serial.println(String("SendHumanDetection dataA->enable : ") + dataA->enable);
                audioMsgQueue.send(dataA);
                neoPixelMsgQueue.send(dataN);
                shuffleMsgQueue.send(dataS);
            }
        }
    });
    wsClient.onErrorReceived([&](uint8_t *payload, size_t length) {
        Serial.println(String(payload, length));
        Serial.println("websocket errorReceived");
    });

    String str = SDUtil::readFile(SDUtil::wifiInfoPath_);

    DynamicJsonDocument doc = DynamicJsonDocument(str.length() * 2);
    deserializeJson(doc, str);

    if (!doc.isNull()) {
        String status = WifiModule::getInstance().connectWifi(doc["ssid"], doc["password"]);
        if (status != "WL_CONNECTED") {
            Serial.println("wifi connect fail");
        }
        else {
            Serial.println("wifi connect");
            wsClient.connect();
        }
    }
    else {
        Serial.println("there is no wifi info");
    }

    webServer.on("/wifi", HTTP_POST, &receiveWifi);
    webServer.begin();

    xTaskCreatePinnedToCore(neoPixelTask, "neoPixelTask", 5000, nullptr, 0, nullptr, 0);
    xTaskCreatePinnedToCore(shuffleTask, "shuffleTask", 5000, nullptr, 0, nullptr, 0);
    xTaskCreatePinnedToCore(audioTask, "audioTask", 100000, nullptr, 1, nullptr, 1);
}

void loop() {
    webServer.handleClient();
    wsClient.loop();
}

void processUserMode(JsonObject data) {
    auto *dataA = new AudioMsgData();
    dataA->list = Playlist();
    dataA->events = AudioMQEvents::UPDATE_ENABLE;
    dataA->enable = false;

    auto *dataN = new NeoPixelMsgData();
    dataN->list = LightEffect();
    dataN->events = NeoPixelMQEvents::UPDATE_ENABLE;
    dataN->mode = ELightMode::None;
    dataN->enable = false;

    auto *dataS = new ShuffleMsgData();
    dataS->events = ShuffleSMQEvents::UPDATE_ENABLE;
    dataS->enable = false;

    UserModeControl::getInstance().interactionMode = Util::stringToEInteractionMode(data["interactionMode"]);
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
    audioMsgQueue.send(dataA);
    neoPixelMsgQueue.send(dataN);
    shuffleMsgQueue.send(dataS);

    UserModeControl::getInstance().operationMode = Util::stringToEOperationMode(data["operationMode"]);

    auto *dataN2 = new NeoPixelMsgData();
    dataN2->list = LightEffect();
    dataN2->events = NeoPixelMQEvents::UPDATE_MODE;
    dataN2->mode = Util::stringToELightMode(data["lightMode"]);

    neoPixelMsgQueue.send(dataN2);

}

void processPlayList(JsonObject data) {

    AudioMsgData *dataA = new AudioMsgData();
    dataA->list = WebSocketClient::parsePlayList(data);
    dataA->events = AudioMQEvents::UPDATE_PLAYLIST;

    audioMsgQueue.send(dataA);
}

void processLightEffects(JsonArray array) {
    for (int i = 0; i < array.size(); i++) {

        NeoPixelMsgData *dataN = new NeoPixelMsgData();
        dataN->list = WebSocketClient::parseLightEffect(array[i]);
        dataN->events = NeoPixelMQEvents::UPDATE_EFFECT;
        dataN->mode = ELightMode::None;

        neoPixelMsgQueue.send(dataN);

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