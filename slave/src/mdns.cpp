
#ifdef ESP32
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#endif

#include "log.h"
#include "config.h"

void setup_mdns()
{
   LOGLN("[mDNS] initializing mDNS service")
   //initialize mDNS service
   ESP_ERROR_CHECK(mdns_init());

   //set hostname
   ESP_ERROR_CHECK(mdns_hostname_set(BONJOUR_NAME));
   ESP_ERROR_CHECK(mdns_instance_name_set("LED_slave"));

   // ESP_ERROR_CHECK(mdns_service_add(nullptr, "_http", "_tcp", 80, nullptr, 0));
   // ESP_ERROR_CHECK(mdns_service_txt_item_set("_http", "_tcp", "path", "/_ac") );

   ESP_ERROR_CHECK(mdns_service_add(nullptr, "_leds", "_tcp", CTRL_PORT, nullptr, 0));

   // ESP_ERROR_CHECK(mdns_service_instance_name_set("_http", "_tcp", "LED Slave: Wifi configuration server"));
   // ESP_ERROR_CHECK(mdns_service_instance_name_set("_ledwall", "_tcp", "LED Slave: control service"));

   LOGLN("[mDNS] mDNS setup done!")
}