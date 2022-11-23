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
  DynamicJsonDocument doc(length);
  deserializeJson(doc, payload);

//  String strJson;
//  serializeJson(doc, strJson);
//  Serial.println(strJson);

  if(doc["event"] == "SendSound") {
    FileInfo fileInfo = {
        .userId = doc["userId"],
        .filename = doc["filename"]
    };
    _fileInfoQueue.push(fileInfo);
  }
}

void WebSocketClient::binaryMessageReceived(uint8_t *payload, size_t length) {
  FileInfo fileInfo = _fileInfoQueue.front();
  _fileInfoQueue.pop();

  Serial.println(fileInfo.filename);
  Serial.println(fileInfo.userId);

//  SDCard::getInstance().writeFile("/" + fileInfo.userId + "/", fileInfo.filename, payload);
  SDCard::getInstance().writeFile("/", "Go High.mp3", payload);
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
  DynamicJsonDocument doc(length);
  deserializeJson(doc, payload);

  String strJson;
  serializeJson(doc, strJson);
  Serial.println(strJson);
}

void WebSocketClient::errorReceived(uint8_t *payload, size_t length) {
  Serial.println("websocket errorReceived");
  DynamicJsonDocument doc(length);
  deserializeJson(doc, payload);

  String strJson;
  serializeJson(doc, strJson);
  Serial.println(strJson);
}
