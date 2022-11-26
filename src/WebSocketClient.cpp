#include "WebSocketClient.h"

class Sound {
public:
  String filename;
  long size;
};

class Playlist {
public:
  long id;
  bool isShuffle;
  std::vector<Sound *> sounds;
};

Playlist playlist;

void parsePlayList(const JsonObject& data) {
  playlist.id = data["id"];
  playlist.isShuffle = data["isShuffle"];
  for (auto sound: playlist.sounds) {
    delete sound;
  }
  for (auto jsonSound: JsonArray(data["sounds"])) {
    auto *sound = new Sound();
    sound->filename = String(jsonSound["filename"]);
    sound->size = jsonSound["size"];

    playlist.sounds.push_back(sound);
  }

//    Serial.println(String("Playlist id: ") + playlist.id);
//    Serial.println(String("Playlist isShuffle: ") + playlist.isShuffle);
//
//
//    for (auto sound: playlist.sounds) {
//        Serial.println(String("Playlist Send filename: ") + sound->filename);
//        Serial.println(String("Playlist Send size: ") + sound->size);
//    }
}

WebSocketClient::WebSocketClient() {
  _client.onEvent([=](WStype_t type, uint8_t * payload, size_t length){
    switch (type) {
      case WStype_ERROR:
        return errorReceived(payload, length);
      case WStype_DISCONNECTED:
        return disconnected(payload, length);
      case WStype_CONNECTED:
        return connected(payload, length);
      case WStype_TEXT:
        return textMessageReceived(payload, length);
      case WStype_BIN:
        return binaryMessageReceived(payload, length);
      default:
        return;
    }
  });
}

void WebSocketClient::connect() {
  if (_withSSL) {
    _client.beginSSL(_host.c_str(), _port, "/socket");
  } else {
    _client.begin(_host.c_str(), _port, "/socket");
  }
}

void WebSocketClient::loop() {
  return _client.loop();
}

bool WebSocketClient::isConnected() {
  return _client.isConnected();
}

void WebSocketClient::setHost(const String &host) {
  _host = host;
}

void WebSocketClient::setPort(int port) {
  _port = port;
}

void WebSocketClient::setWithSsl(bool withSsl) {
  _withSSL = withSsl;
}

void WebSocketClient::textMessageReceived(uint8_t *payload, size_t length) {
  DynamicJsonDocument doc(length * 2);
  deserializeJson(doc, payload);

  if (doc.containsKey("authenticationToken")) {
    SDUtil::authenticationToken_ = String(doc["authenticationToken"]);
    parsePlayList(doc["playlist"]);
  }
  else if (doc["event"] == "SendLightEffect") {
  }
  else if (doc["event"] == "SendSound") {
    String protocol = _withSSL ? "https://" : "http://";
    long id = doc["id"];
    String filename = doc["filename"];
    String url = protocol + _host + ":" + _port + "/api/file/" + filename;

    SDUtil::writeFile(url, id, filename);
  }
  else if (doc["event"] == "SendPlaylist") {
    parsePlayList(doc["data"]);
  }
}

void WebSocketClient::binaryMessageReceived(uint8_t *payload, size_t length) {
}

void WebSocketClient::connected(uint8_t *payload, size_t length) {
  Serial.println("websocket connected");

  DynamicJsonDocument doc(512);
  JsonObject json = doc.to<JsonObject>();
  json["event"] = "registerDeviceSession";
  json["name"] = "Kira";
  json["type"] = "Mirror";

  String strJson;
  serializeJson(json, strJson);

  _client.sendTXT(strJson.c_str());
}

void WebSocketClient::disconnected(uint8_t *payload, size_t length) {
  Serial.println("websocket disconnected");
}

void WebSocketClient::errorReceived(uint8_t *payload, size_t length) {
  Serial.println("websocket errorReceived");
}