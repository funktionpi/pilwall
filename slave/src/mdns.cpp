
#ifdef ESP32
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#endif

#include "config.h"

void setup_mdns()
{
   //initialize mDNS service
   esp_err_t err = mdns_init();
   if (err)
   {
      printf("[MDNS] Init failed: %d\n", err);
      return;
   }

   //set hostname
   mdns_hostname_set(BONJOUR_NAME);
   mdns_instance_name_set("led-slave driver");

   mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
   mdns_service_add(NULL, "_ledwall", "_tcp", CTRL_PORT, NULL, 0);

   mdns_service_instance_name_set("_http", "_tcp", "led-slave's Wifi configuration server");
   mdns_service_instance_name_set("_ledwall", "_tcp", "led-slave's control service");
}