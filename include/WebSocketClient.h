#ifndef MILKYWAY_WEB_SOCKET_CLIENT_H
#define MILKYWAY_WEB_SOCKET_CLIENT_H

#include "Singleton.h"
#include "WebSocketsClient.h"

#include <ArduinoJson.h>
#include <WString.h>
#include <functional>
#include <optional>

class WebSocketClient{
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

    typedef std::function<void(uint8_t *, size_t)> webSocketReceiveCB;

    void onTextMessageReceived(webSocketReceiveCB cb){_webSockectReceiveText = cb;};

    void onBinaryMessageReceived(webSocketReceiveCB cb){_webSockectReceiveBinary = cb;};

    void onConnected(webSocketReceiveCB cb){_webSockectReceiveConnected = cb;};

    void onDisconnected(webSocketReceiveCB cb){_webSockectReceiveDisconnected = cb;};

    void onErrorReceived(webSocketReceiveCB cb){_webSockectReceiveError = cb;};

private:
    String _host = "0.0.0.0";
    int _port = 80;
    bool _withSSL = false;

    WebSocketsClient _client;

    std::optional<webSocketReceiveCB> _webSockectReceiveText;
    std::optional<webSocketReceiveCB> _webSockectReceiveBinary;
    std::optional<webSocketReceiveCB> _webSockectReceiveConnected;
    std::optional<webSocketReceiveCB> _webSockectReceiveDisconnected;
    std::optional<webSocketReceiveCB> _webSockectReceiveError;
};


#endif //MILKYWAY_WEBSOCKETCLIENT_H
