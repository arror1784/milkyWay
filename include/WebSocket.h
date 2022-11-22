//
// Created by jepanglee on 2022-11-22.
//

#ifndef MILKYWAY_WEBSOCKET_H
#define MILKYWAY_WEBSOCKET_H

#include "header.h"

class WebSocket {
public:
  void connect(const String &host, int port, bool withSSL);

  bool isConnected();

private:
  void textMessageReceived(uint8_t *payload, size_t length);

  void binaryMessageReceived(uint8_t *payload, size_t length);

  void connected(uint8_t *payload, size_t length);

  void disconnected(uint8_t *payload, size_t length);

  void errorReceived(uint8_t *payload, size_t length);

  String _uri;

  WebSocketsClient _client;
};


#endif //MILKYWAY_WEBSOCKET_H
