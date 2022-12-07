#include "WebSocketClient.h"

WebSocketClient::WebSocketClient() {
    _client.onEvent([=](WStype_t type, uint8_t *payload, size_t length) {
        switch (type) {
            case WStype_ERROR:
                if (_webSocketReceiveError.has_value()) _webSocketReceiveError.value()(payload, length);
                return;
            case WStype_DISCONNECTED:
                if (_webSocketReceiveDisconnected.has_value())_webSocketReceiveDisconnected.value()(payload, length);
                return;
            case WStype_CONNECTED:
                if (_webSocketReceiveConnected.has_value()) _webSocketReceiveConnected.value()(payload, length);
                return;
            case WStype_TEXT:
                if (_webSocketReceiveText.has_value()) _webSocketReceiveText.value()(payload, length);
                return;
            case WStype_PING:
                if (_webSocketReceivePing.has_value()) _webSocketReceivePing.value()(payload, length);
                return;
            default:
                return;
        }
    });
    _client.setReconnectInterval(2500);
}

void WebSocketClient::connect() {
    if (_withSSL) {
        _client.beginSSL(_host.c_str(), _port, "/socket");
    }
    else {
        _client.begin(_host.c_str(), _port, "/socket");
    }
}

void WebSocketClient::disconnect() {
    _client.disconnect();
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

void WebSocketClient::sendText(String txt) {
    _client.sendTXT(txt.c_str());
}

void WebSocketClient::sendPong() {
    _client.sendPing();
}
