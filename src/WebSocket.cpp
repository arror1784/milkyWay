//
// Created by jepanglee on 2022-11-22.
//

#include "WebSocket.h"

void WebSocket::connect(const String &host, int port, bool withSSL) {
  if (withSSL) {
    _client.beginSSL(host.c_str(), port);
  } else {
    _client.begin(host.c_str(), port);
  }
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

void WebSocket::textMessageReceived(uint8_t *payload, size_t length) {

}

void WebSocket::binaryMessageReceived(uint8_t *payload, size_t length) {

}

void WebSocket::connected(uint8_t *payload, size_t length) {
  Serial.println("websocket connected");

  JsonObject json = JsonObject();
  json["event"] = "registerDeviceSession";
  json["name"] = "Kira";
  json["type"] = "Mirror";

  String strJson;
  DynamicJsonDocument jsonDoc = DynamicJsonDocument(json);
  serializeJson(jsonDoc, strJson);

  _client.sendTXT(strJson.c_str());
}

void WebSocket::disconnected(uint8_t *payload, size_t length) {

}

void WebSocket::errorReceived(uint8_t *payload, size_t length) {
  DynamicJsonDocument doc(length);
  deserializeJson(doc, payload);
  String strJson;
  serializeJson(doc, strJson);

  Serial.println(strJson.c_str());
}

bool WebSocket::isConnected() {
  return _client.isConnected();
}
