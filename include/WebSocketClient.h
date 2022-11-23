//
// Created by jepanglee on 2022-11-22.
//

#ifndef MILKYWAY_WEB_SOCKET_CLIENT_H
#define MILKYWAY_WEB_SOCKET_CLIENT_H

#include "SDCard.h"

struct FileInfo {
  String userId;
  String filename;
};

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

  std::queue<FileInfo> _fileInfoQueue;
};


#endif //MILKYWAY_WEBSOCKETCLIENT_H
