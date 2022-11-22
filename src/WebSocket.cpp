//
// Created by jepanglee on 2022-11-22.
//

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

void WebSocketClient::connect(const String &host, int port, bool withSSL) {
  if (withSSL) {
    _client.beginSSL(host.c_str(), port, "/socket");
  } else {
    _client.begin(host.c_str(), port, "/socket");
  }
}

void WebSocketClient::textMessageReceived(uint8_t *payload, size_t length) {
  DynamicJsonDocument doc(length);
  deserializeJson(doc, payload);

  String strJson;
  serializeJson(doc, strJson);
  Serial.println(strJson);

  if(doc["event"] == "SendSound") {
    _fileInfoQueue.push({
      .userId = doc["userId"],
      .filename = doc["filename"]
    });
  }
}

void WebSocketClient::binaryMessageReceived(uint8_t *payload, size_t length) {
  FileInfo fileInfo = _fileInfoQueue.front();
  _fileInfoQueue.pop();

  SDCard::getInstance().writeFile(fileInfo.userId + "/", fileInfo.filename, payload);
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

}

bool WebSocketClient::isConnected() {
  return _client.isConnected();
}

void WebSocketClient::loop() {
  return _client.loop();
}
