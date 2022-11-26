#include "MdnsModule.h"

MdnsModule::MdnsModule() {
}

MdnsModule::~MdnsModule() {

}

void MdnsModule::mDnsInit(std::string name) {
  _hostName = name;

  //initialize mDNS
  ESP_ERROR_CHECK( mdns_init() );
  //set mDNS hostname (required if you want to advertise services)
  ESP_ERROR_CHECK( mdns_hostname_set(_hostName.data()) );
  Serial.printf("mdns hostname set to: [%s]\r\n", _hostName.data());
  //set default mDNS instance name
  ESP_ERROR_CHECK( mdns_instance_name_set("CONFIG_MDNS_INSTANCE") );

  mdns_txt_item_t serviceTxtData[3] = {
    {"board", "esp32"},
    {"u", "user"},
    {"p", "password"}
  };
  ESP_ERROR_CHECK( mdns_service_add("ESP32-WebServer", "_milkyWat", "_tcp", 3232, serviceTxtData, 3) );
  // ESP_ERROR_CHECK( mdns_service_instance_name_set("_http", "_tcp", "MilkyWay's ESP32 Web Server"));

}