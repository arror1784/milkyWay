
// #ifndef MILKYWAY_SIMPLEWEBSERVER_H
// #define MILKYWAY_SIMPLEWEBSERVER_H

// #include <Arduino.h>
// #include <ArduinoJson.h>

// #include <ESPAsyncWebServer.h>
// #include <RichHttpServer.h>

// #include "Singleton.h"

// #define SERVER_PORT 80

// using RichHttpConfig = RichHttp::Generics::Configs::AsyncWebServer;
// using RequestContext = RichHttpConfig::RequestContextType;

// class SimpleWebServer : public Singleton<SimpleWebServer>{

// public:
//     SimpleWebServer();

//     void addHandler(std::string uri, RichHttpConfig::HttpMethod method, std::function<void(RequestContext&)> fn);
    
//     void start();
// private:
//     SimpleAuthProvider *_authProvider;
//     RichHttpServer<RichHttpConfig> *_server;
// };

// #endif