
// #include "SimpleWebServer.h"

// SimpleWebServer::SimpleWebServer(){
//      _authProvider = new SimpleAuthProvider();
//      _server = new RichHttpServer<RichHttpConfig>(SERVER_PORT,*_authProvider);
// }

// void SimpleWebServer::addHandler(std::string uri, RichHttpConfig::HttpMethod method, std::function<void(RequestContext&)> fn){

//     _server
//         ->buildHandler(uri.c_str())
//         .on(method, fn);

// }

// void SimpleWebServer::start(){

//     _server->clearBuilders();
//     _server->begin();
// }
