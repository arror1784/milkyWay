#include <Arduino.h>

#include "asio.hpp"

#include <ESP32-audioI2S/Audio.h>

#include "WifiModule.h"
#include "MdnsModule.h"
#include "nvs_flash.h"

#include <string.h>

#define I2S_DOUT      6
#define I2S_BCLK      7
#define I2S_LRC       8

void setup() {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  Serial.begin(115200);
  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  WifiModule::getInstance().connect("JSH","00000000");
  // WifiModule::getInstance().

  // MdnsModule::getInstance().mDnsInit("Hello world");

}

void loop() {
}