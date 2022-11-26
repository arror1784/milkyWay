#include "WebSocketClient.h"

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
  NeoPixel &neoPixel = NeoPixel::getInstance();

  DynamicJsonDocument doc(length * 2);
  deserializeJson(doc, payload);

  if (doc.containsKey("authenticationToken")) {
    SDUtil::authenticationToken_ = String(doc["authenticationToken"]);
    neoPixel.setLightEffects(doc["lightEffects"]);
    neoPixel.setLightEffectId(Util::stringToELightMode(doc["userMode"]["lightMode"]));
    neoPixel.setRandomColorSetId();
  }
  else if (doc["event"] == "SendLightEffect") {
    neoPixel.setLightEffect(doc["data"]);
  }
  else if (doc["event"] == "SendSound") {
    String protocol = _withSSL ? "https://" : "http://";
    long id = doc["id"];
    String filename = doc["filename"];
    String url = protocol + _host + ":" + _port + "/api/file/" + filename;

    SDUtil::writeFile(url, id, filename);
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