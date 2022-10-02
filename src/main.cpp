#include <Arduino.h>

#include <string.h>
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <ESP32-audioI2S/Audio.h>

#define I2S_DOUT      6
#define I2S_BCLK      7
#define I2S_LRC       8

#define EXAMPLE_ESP_WIFI_SSID      "MILKY_WAY_SSID"
#define EXAMPLE_ESP_WIFI_PASS      "00000000"
#define EXAMPLE_ESP_WIFI_CHANNEL   3
#define EXAMPLE_MAX_STA_CONN       3

static const char *TAG = "wifi softAP";

int cur_volume = 20;

const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
  if (event_id == WIFI_EVENT_AP_STACONNECTED) {
    wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
    ESP_LOGI(TAG, "station ",MACSTR," join, AID=%d",
             MAC2STR(event->mac), event->aid);
  } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
    ESP_LOGI(TAG, "station ",MACSTR," leave, AID=%d",
             MAC2STR(event->mac), event->aid);
  }
}


void wifi_init_softap(void)
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      NULL));

  wifi_config_t wifi_config;
  memset(&wifi_config.ap,0,sizeof(wifi_config.ap));

  memcpy(wifi_config.ap.ssid,EXAMPLE_ESP_WIFI_SSID,strlen(EXAMPLE_ESP_WIFI_SSID));
  wifi_config.ap.ssid_len = (uint8_t)strlen(EXAMPLE_ESP_WIFI_SSID),
  memcpy(wifi_config.ap.password,EXAMPLE_ESP_WIFI_PASS,strlen(EXAMPLE_ESP_WIFI_PASS));
  wifi_config.ap.channel = EXAMPLE_ESP_WIFI_CHANNEL,
  wifi_config.ap.max_connection = EXAMPLE_MAX_STA_CONN,
  wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK,
  wifi_config.ap.ssid_hidden = 0,
  wifi_config.ap.beacon_interval = 100,
  wifi_config.ap.pairwise_cipher = WIFI_CIPHER_TYPE_TKIP,
  wifi_config.ap.ftm_responder = true;

  if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
           EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

void setup() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
  wifi_init_softap();
}

void loop() {
// write your code here

//  WiFiClient client = server.available(); // client 시작
//    if(client.available()) {
//      income_wifi = client.readStringUntil('\r');
//    Serial.println(income_wifi);
//  }
//  client.stop();
}