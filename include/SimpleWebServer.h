
#ifndef MILKYWAY_SIMPLEWEBSERVER_H
#define MILKYWAY_SIMPLEWEBSERVER_H

#include <esp_http_server.h>

#include "Singleton.h"
#include <vector>
#include <string>
#include <functional>

typedef esp_err_t httpUriHandler_t( httpd_req_t* ) ;

httpd_handle_t simpleServerStart();
esp_err_t simpleServerStop(httpd_handle_t server);

#endif