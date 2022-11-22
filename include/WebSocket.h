//
// Created by jepanglee on 2022-11-22.
//

#ifndef MILKYWAY_WEBSOCKET_H
#define MILKYWAY_WEBSOCKET_H

#include "header.h"

class WebSocket{
public:
  void init();
  void connect();
  void disconnect();
  void setUri(String uri);
private:
  String _uri;

  WebSocketsClient _client;
};


#endif //MILKYWAY_WEBSOCKET_H
