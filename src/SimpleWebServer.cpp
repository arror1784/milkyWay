
#include "SimpleWebServer.h"

#include <Arduino.h>
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "WifiModule.h"
static unsigned int visitors = 0;

static void connect_free_func(void *ctx)
{
    ESP_LOGI(TAG, "/connect Free Context function called");
    free(ctx);
}
static esp_err_t test(httpd_req_t *req){

    /* Send a simple response */
    const char resp[] = "URI GET TTTTTTTTTTTTTTTTTTTTTTTTTEST";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
} 

static const httpd_uri_t test_get = {
    .uri      = "/test",
    .method   = HTTP_GET,
    .handler  = test,
    .user_ctx = &visitors
};

static esp_err_t connectToAPHandler(httpd_req_t *req){
    /* Log total visitors */
    char buf[100] ={0};
    char outbuf[50];
    int  ret;
    unsigned int *visitors = (unsigned int *)req->user_ctx;
    rapidjson::Document doc;

    /* Read data received in the request */
    ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    /* Create session's context if not already available */
    if (! req->sess_ctx) {
        ESP_LOGI(TAG, "/adder PUT allocating new session");
        req->sess_ctx = malloc(sizeof(unsigned int));
        req->free_ctx = NULL;
    }
    *(int *)req->sess_ctx = *visitors;


    Serial.println(buf);

    if(doc.Parse(buf).HasParseError()){
        httpd_resp_send_408(req);
        return ESP_OK;
    }

    if(!doc.IsObject() || !doc.HasMember("ssid") || !doc.HasMember("password")){
        httpd_resp_send_408(req);
        return ESP_OK;
    }
    if(!doc["ssid"].IsString() || !doc["password"].IsString()){
        httpd_resp_send_408(req);
        return ESP_OK;
    }
    Serial.print("ssid : ");
    Serial.println(doc["ssid"].GetString());
    Serial.print("passwd : ");
    Serial.println(doc["password"].GetString());

    if(WifiModule::getInstance().connect(doc["ssid"].GetString(),doc["password"].GetString())){
        httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
    }else{
        httpd_resp_send_500(req);
    }

    /* Respond with the reset value */
    // snprintf(outbuf, sizeof(outbuf),"%d", *((int *)req->sess_ctx));
    return ESP_OK;
} 

static const httpd_uri_t connectToAP = {
    .uri      = "/connect",
    .method   = HTTP_POST,
    .handler  = connectToAPHandler,
    .user_ctx = &visitors
};
static esp_err_t disconnectToAPHandler(httpd_req_t *req){
    /* Log total visitors */
    char buf[100] ={0};
    int  ret;
    unsigned int *visitors = (unsigned int *)req->user_ctx;

    /* Read data received in the request */
    ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    /* Create session's context if not already available */
    if (! req->sess_ctx) {
        ESP_LOGI(TAG, "/adder PUT allocating new session");
        req->sess_ctx = malloc(sizeof(unsigned int));
        req->free_ctx = NULL;
    }
    *(int *)req->sess_ctx = *visitors;

    Serial.println(buf);

    WifiModule::getInstance().disconnect();

    /* Respond with the reset value */
    // snprintf(outbuf, sizeof(outbuf),"%d", *((int *)req->sess_ctx));
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
} 

static const httpd_uri_t disconnectToAP = {
    .uri      = "/disconnect",
    .method   = HTTP_POST,
    .handler  = disconnectToAPHandler,
    .user_ctx = &visitors
};

static esp_err_t apListHandler(httpd_req_t *req){
    /* Log total visitors */
    WifiModule::getInstance().getApList();

    /* Respond with the reset value */
    httpd_resp_send(req, "asdasdsda", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
static const httpd_uri_t apList = {
    .uri      = "/aplist",
    .method   = HTTP_GET,
    .handler  = apListHandler,
    .user_ctx = &visitors
};

httpd_handle_t simpleServerStart(){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // Start the httpd server
    Serial.printf("Starting server on port: '%d'\r\n", config.server_port);
    httpd_handle_t server;

    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
         Serial.printf("Registering URI handlers\r\n");
        httpd_register_uri_handler(server, &test_get);
        httpd_register_uri_handler(server, &connectToAP);
        httpd_register_uri_handler(server, &disconnectToAP);
        httpd_register_uri_handler(server, &apList);


        return server;
    }

    Serial.printf("httpd_start fail\r\n");
    return NULL;
}
esp_err_t simpleServerStop(httpd_handle_t server){
    return httpd_stop(server);
}