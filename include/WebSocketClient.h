#ifndef MILKYWAY_WEB_SOCKET_CLIENT_H
#define MILKYWAY_WEB_SOCKET_CLIENT_H

#include "Singleton.h"
#include "WebSocketsClient.h"
#include "NeoPixel.h"
#include "SDUtil.h"
#include "AudioControl.h"

#include <ArduinoJson.h>
#include <WString.h>
#include <functional>
#include <optional>

class WebSocketClient {
public:
    WebSocketClient();

    void connect();

    void disconnect();

    void loop();

    bool isConnected();

    void setHost(const String &host);

    void setPort(int port);

    void setWithSsl(bool withSsl);

    void sendText(String txt);

    void sendPong();

    typedef std::function<void(uint8_t *, size_t)> webSocketReceiveCB;

    void onTextMessageReceived(webSocketReceiveCB cb) { _webSocketReceiveText = cb; };

    void onPingMessageReceived(webSocketReceiveCB cb) { _webSocketReceivePing = cb; };

    void onConnected(webSocketReceiveCB cb) { _webSocketReceiveConnected = cb; };

    void onDisconnected(webSocketReceiveCB cb) { _webSocketReceiveDisconnected = cb; };

    void onErrorReceived(webSocketReceiveCB cb) { _webSocketReceiveError = cb; };

    static Playlist parsePlayList(const JsonObject &data) {
        Playlist playlist;
        playlist.id = data["id"];
        playlist.isShuffle = data["isShuffle"];
        playlist.sounds.clear();
        for (auto jsonSound: JsonArray(data["sounds"])) {
            Sound sound;
            sound.filename = String(jsonSound["filename"]);
            sound.id = jsonSound["id"];

            playlist.sounds.push_back(sound);
        }
        return playlist;
    }

    static LightEffect parseLightEffect(const JsonObject &jsonLightEffect) {
        LightEffect lightEffect;

        lightEffect.id = jsonLightEffect["id"];
        lightEffect.mode = Util::stringToELightMode(jsonLightEffect["mode"]);
        lightEffect.isRandomColor = jsonLightEffect["randomColor"];
        lightEffect.speed = jsonLightEffect["speed"];
        lightEffect.isRandomSpeed = jsonLightEffect["randomSpeed"];

        for (JsonObject jsonColorSet: JsonArray(jsonLightEffect["colors"])) {
            ColorSet colorSet;

            colorSet.id = jsonColorSet["id"];

            for (String color: JsonArray(jsonColorSet["colors"])) {
                colorSet.colors.push_back(Util::stringToRGBW(color));
            }
            lightEffect.colorSets.push_back(colorSet);
        }

        return lightEffect;
    }

private:
    String _host = "0.0.0.0";
    int _port = 80;
    bool _withSSL = false;

    WebSocketsClient _client;

    std::optional<webSocketReceiveCB> _webSocketReceiveText;
    std::optional<webSocketReceiveCB> _webSocketReceivePing;
    std::optional<webSocketReceiveCB> _webSocketReceiveConnected;
    std::optional<webSocketReceiveCB> _webSocketReceiveDisconnected;
    std::optional<webSocketReceiveCB> _webSocketReceiveError;
};


#endif //MILKYWAY_WEBSOCKETCLIENT_H
