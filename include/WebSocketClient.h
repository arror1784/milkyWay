#ifndef MILKYWAY_WEB_SOCKET_CLIENT_H
#define MILKYWAY_WEB_SOCKET_CLIENT_H

#include "Singleton.h"
#include "WebSocketsClient.h"
#include "NeoPixel.h"
#include "SDUtil.h"

#include <ArduinoJson.h>
#include <WString.h>
#include <queue>

class WebSocketClient : public Singleton<WebSocketClient> {
public:
  WebSocketClient();

  void connect();

  void loop();

  bool isConnected();

  void setHost(const String &host);

  void setPort(int port);

  void setWithSsl(bool withSsl);

private:
  void textMessageReceived(uint8_t *payload, size_t length);

  void binaryMessageReceived(uint8_t *payload, size_t length);

  void connected(uint8_t *payload, size_t length);

  void disconnected(uint8_t *payload, size_t length);

  void errorReceived(uint8_t *payload, size_t length);

private:
  String _host = "0.0.0.0";
  int _port = 80;
  bool _withSSL = false;

  WebSocketsClient _client;
};


#endif //MILKYWAY_WEBSOCKETCLIENT_H
