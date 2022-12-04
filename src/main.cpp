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
#include "AudioDownloadMsgQueue.h"
#include "NeoPixelMsgQueue.h"
#include "ShuffleMsgQueue.h"

#define LED_PIN 32
#define LED_LENGTH 24

#define I2S_DOUT      2
#define I2S_BCLK      0
#define I2S_LRC       4

#define SUFFLE_TIMEOUT 5500

static const String host = "192.168.0.195";
static const int port = 6001;
static const bool ssl = false;

//static const String host = "kira-api.wimcorp.dev";
//static const int port = 443;
//static const bool ssl = true;
static const int syncUpdateResolution = 10;

NeoPixelMsgQueue neoPixelMsgQueue(5);

void neoPixelTask(void *params) {
    Neopixel neopixel(LED_LENGTH, LED_PIN, NEO_GRBW | NEO_KHZ800);

    while (1) {
        NeoPixelMsgData *msg = neoPixelMsgQueue.recv();
        if (msg != nullptr) {
            if (msg->events == NeoPixelMQEvents::UPDATE_EFFECT) {
                neopixel.setLightEffect(msg->lightEffect);
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

int volume = 10;

void audioTask(void *params) {
    audioControl.setVolume(volume); // 0...21
    TickType_t tick = xTaskGetTickCount();

    while (1) {
        AudioMsgData *msg = audioMsgQueue.recv();

        if (audioControl.isDownloading()) {
            if (msg != nullptr) delete msg;

            continue;
        }
        if (msg != nullptr) {
            if (msg->events == AudioMQEvents::UPDATE_PLAYLIST) {
                Serial.println("updatePLAYLIST");
                auto &list = msg->list;
                audioControl.setPlayList(list);
            }
            else if (msg->events == AudioMQEvents::UPDATE_ENABLE) {
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
                dataN->lightEffect = LightEffect();
                dataN->events = NeoPixelMQEvents::UPDATE_SYNC;
                dataN->mode = ELightMode::None;
                auto absData = std::abs(audioControl.getLastGain()) / volume;
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
            dataN->lightEffect = LightEffect();
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

void processUserMode(const JsonObject &data) {
    auto *dataA = new AudioMsgData();
    dataA->list = Playlist();
    dataA->events = AudioMQEvents::UPDATE_ENABLE;
    dataA->enable = false;

    auto *dataN = new NeoPixelMsgData();
    dataN->lightEffect = LightEffect();
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
    dataN2->lightEffect = LightEffect();
    dataN2->events = NeoPixelMQEvents::UPDATE_MODE;
    dataN2->mode = Util::stringToELightMode(data["lightMode"]);

    neoPixelMsgQueue.send(dataN2);

}

void processPlayList(const JsonObject &data) {
    auto dataA = new AudioMsgData();
    dataA->list = WebSocketClient::parsePlayList(data);
    dataA->events = AudioMQEvents::UPDATE_PLAYLIST;

    audioMsgQueue.send(dataA);
}

void processLightEffects(const JsonArray &array) {
    bool isBlinkingExists = false;
    bool isBreathingExists = false;
    bool isColorChangeExists = false;

    for (auto jsonLightEffect: array) {
        auto *dataN = new NeoPixelMsgData();

        dataN->lightEffect = WebSocketClient::parseLightEffect(jsonLightEffect);
        dataN->events = NeoPixelMQEvents::UPDATE_EFFECT;
        dataN->mode = ELightMode::None;

        neoPixelMsgQueue.send(dataN);

        if (dataN->mode == ELightMode::Blinking) isBlinkingExists = true;
        else if (dataN->mode == ELightMode::Breathing) isBreathingExists = true;
        else if (dataN->mode == ELightMode::ColorChange) isColorChangeExists = true;
    }

    if (isBlinkingExists) {
        auto *dataN = new NeoPixelMsgData();

        dataN->lightEffect.mode = ELightMode::Blinking;
        dataN->events = NeoPixelMQEvents::UPDATE_EFFECT;
        dataN->mode = ELightMode::None;

        neoPixelMsgQueue.send(dataN);
    }
    if (isBreathingExists) {
        auto *dataN = new NeoPixelMsgData();

        dataN->lightEffect.mode = ELightMode::Breathing;
        dataN->events = NeoPixelMQEvents::UPDATE_EFFECT;
        dataN->mode = ELightMode::None;

        neoPixelMsgQueue.send(dataN);
    }
    if (isColorChangeExists) {
        auto *dataN = new NeoPixelMsgData();

        dataN->lightEffect.mode = ELightMode::ColorChange;
        dataN->events = NeoPixelMQEvents::UPDATE_EFFECT;
        dataN->mode = ELightMode::None;

        neoPixelMsgQueue.send(dataN);
    }
}

WebSocketClient wsClient;
WebServer webServer(80);

bool connectWifiBySdData() {
    String str = SDUtil::readFile(SDUtil::wifiInfoPath_);

    DynamicJsonDocument doc = DynamicJsonDocument(str.length() * 2);
    deserializeJson(doc, str);

    if (doc.isNull()) {
        Serial.println("there is no wifi info");
        return false;
    }

    Serial.print("ssid : ");
    Serial.println(String(doc["ssid"]));
    Serial.print("password : ");
    Serial.println(String(doc["password"]));

    String status = WifiModule::getInstance().connectWifi(doc["ssid"], doc["password"]);

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

        bool status = connectWifiBySdData();

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

bool isFirstConnection = true;
AudioDownloadMsgQueue audioDownloadMsgQueue(5);

void setup() {
    Serial.begin(115200);

    SDUtil::getInstance().init();
    WiFiClass::mode(WIFI_MODE_STA);
    Serial.println(SDUtil::getInstance().getSerial());
    WifiModule::getInstance().setIp("192.168.0.1", "192.168.0.1", "255.255.255.0");
    WifiModule::getInstance().setApInfo(SDUtil::getInstance().getSerial());

    webServer.on("/wifi", HTTP_POST, &receiveWifi);
    webServer.begin();

    wsClient.setHost(host);
    wsClient.setPort(port);
    wsClient.setWithSsl(ssl);
    wsClient.onConnected([&](uint8_t *payload, size_t length) {
        Serial.println("websocket connected");

        DynamicJsonDocument doc(512);
        JsonObject json = doc.to<JsonObject>();
        json["event"] = "registerDeviceSession";
        json["name"] = SDUtil::getInstance().getSerial();
        json["isFirstConnection"] = isFirstConnection;
        Serial.println(String(json["name"]));
        json["type"] = "Mirror";

        String strJson;
        serializeJson(json, strJson);

        wsClient.sendText(strJson);

        isFirstConnection = false;
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
                dataA->events = AudioMQEvents::UPDATE_ENABLE;
                dataA->enable = false;

                auto *dataN = new NeoPixelMsgData();
                dataN->lightEffect = LightEffect();
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

    xTaskCreatePinnedToCore(neoPixelTask, "neoPixelTask", 5000, nullptr, 0, nullptr, 0);
    xTaskCreatePinnedToCore(shuffleTask, "shuffleTask", 5000, nullptr, 0, nullptr, 0);
    xTaskCreatePinnedToCore(audioTask, "audioTask", 10000, nullptr, 1, nullptr, 1);
    connectWifiBySdData();
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
        connectWifiBySdData();
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