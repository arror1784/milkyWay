#include "WebSocketClient.h"

WebSocketClient::WebSocketClient() {
    _client.onEvent([=](WStype_t type, uint8_t * payload, size_t length){
        switch (type) {
            case WStype_ERROR:
                if(_webSockectReceiveError.has_value()) _webSockectReceiveError.value()(payload,length);
                return;
            case WStype_DISCONNECTED:
                if(_webSockectReceiveDisconnected.has_value()) _webSockectReceiveDisconnected.value()(payload,length);
                return;
            case WStype_CONNECTED:
                if(_webSockectReceiveConnected.has_value()) _webSockectReceiveConnected.value()(payload,length);
                return;
            case WStype_TEXT:
                if(_webSockectReceiveText.has_value()) _webSockectReceiveText.value()(payload,length);
                return;
            case WStype_BIN:
                if(_webSockectReceiveBinary.has_value()) _webSockectReceiveBinary.value()(payload,length);
                return;
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

void WebSocketClient::sendText(String txt){
    _client.sendTXT(txt.c_str());
}
