//
// Created by jepanglee on 2022-10-03.
//
#include <Arduino.h>

#include "string.h"

#include "WifiModule.h"

WifiModule::WifiModule() {
  initStaMode();
  initApMode(_SSID,_PASSWD);
  ESP_ERROR_CHECK(esp_wifi_start());
}
WifiModule::~WifiModule() {
}

void WifiModule::initApMode(std::string ssid,std::string passwd) {

  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &wifi_ap_event_handler,
                                                      NULL,
                                                      NULL));

  wifi_config_t wifi_config;
  memset(&wifi_config.ap,0,sizeof(wifi_config.ap));

  memcpy(wifi_config.ap.ssid,ssid.data(),ssid.length());
  wifi_config.ap.ssid_len = ssid.length(),
  memcpy(wifi_config.ap.password,passwd.data(),passwd.length());
  wifi_config.ap.channel = 0,
  wifi_config.ap.max_connection = 5,
  wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK,
  wifi_config.ap.ssid_hidden = 0,
  wifi_config.ap.beacon_interval = 100,
  wifi_config.ap.pairwise_cipher = WIFI_CIPHER_TYPE_TKIP,
  wifi_config.ap.ftm_responder = true;

  if (passwd.length() == 0) {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA) );
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

  Serial.printf("wifi_init_softap finished. SSID:%s password:%s channel:%d\r\n",ssid.data(), passwd.data(), 0);

}

void WifiModule::initStaMode() {

  s_wifi_event_group = xEventGroupCreate();
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &wifi_sta_event_handler,
                                                      NULL,
                                                      &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &wifi_sta_event_handler,
                                                      NULL,
                                                      &instance_got_ip));

  Serial.printf("wifi_init_sta finished.\r\n");

}
bool WifiModule::connect(std::string ssid, std::string passwd) {

  wifi_config_t wifi_config;

  memset(&wifi_config.sta,0,sizeof(wifi_config.sta));
  memcpy(wifi_config.sta.ssid,ssid.data(),ssid.length());
  memcpy(wifi_config.sta.password,passwd.data(),passwd.length());
//  wifi_config.sta.bssid
  wifi_config.sta.bssid_set = false;
  wifi_config.sta.channel = 0;
  wifi_config.sta.btm_enabled = 0;
  wifi_config.sta.rm_enabled = 0;
  wifi_config.sta.mbo_enabled = 0;
  wifi_config.sta.listen_interval = 0;
//  wifi_config.sta.pmf_cfg =
//  wifi_config.sta.reserved =
  wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );


  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
   * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE,
                                         pdFALSE,
                                         portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
   * happened. */
  if (bits & WIFI_CONNECTED_BIT) {
    Serial.printf("connected to ap SSID:%s password:%s\r\n", ssid.data(), passwd.data());
  } else if (bits & WIFI_FAIL_BIT) {
    Serial.printf("Failed to connect to SSID:%s, password:%s\r\n", ssid.data(), passwd.data());
  } else {
    Serial.printf("UNEXPECTED EVENT");
  }
  return true;
}

void WifiModule::disconnect() {
  esp_wifi_disconnect();
  esp_wifi_clear_fast_connect();
}

void WifiModule::setApSetting(wifi_config_t wifi_config) {
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config) );
}

void WifiModule::setStaSetting(wifi_config_t wifi_config) {
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
}



void wifi_ap_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data){
  if (event_id == WIFI_EVENT_AP_STACONNECTED) {
    wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
    Serial.printf("AP ");
    Serial.printf(MACSTR, MAC2STR(event->mac));
    Serial.printf(" join, AID=%d\r\n", event->aid);

  } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
    Serial.printf("AP ");
    Serial.printf(MACSTR, MAC2STR(event->mac));
    Serial.printf(" leave, AID=%d\r\n", event->aid);
  }
}

void wifi_sta_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data){
  static int s_retry_num = 0;

  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < ESP_MAXIMUM_RETRY) {
      esp_wifi_connect();
      s_retry_num++;
      Serial.printf("retry to connect to the AP\r\n");
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    Serial.printf("connect to the AP fail\r\n");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    Serial.printf("got ip:");
    Serial.printf(IPSTR, IP2STR(&event->ip_info.ip));
    Serial.printf("\r\n");
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}
