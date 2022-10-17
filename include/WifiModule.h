//
// Created by jepanglee on 2022-10-03.
//

#ifndef MILKYWAY_WIFIMODULE_H
#define MILKYWAY_WIFIMODULE_H

#include "Singleton.h"

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define ESP_MAXIMUM_RETRY  5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

static EventGroupHandle_t s_wifi_event_group;

class WifiModule : public Singleton<WifiModule>{

public:
  WifiModule();
  ~WifiModule();

  void initApMode(std::string,std::string);
  void initStaMode();

  bool connect(std::string ssid, std::string passwd);
  void disconnect();

  void setApSetting(wifi_config_t wifi_config);
  void setStaSetting(wifi_config_t wifi_config);

private:
  const std::string _SSID = "MILKY_WAY";
  const std::string _PASSWD = "00000000";
};

void wifi_ap_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data);
void wifi_sta_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data);

#endif //MILKYWAY_WIFIMODULE_H
