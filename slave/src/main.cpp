#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>

#include "config.h"
#include "ledslave.h"
#include "led.h"
#include "log.h"

void setup()
{
#ifdef DEBUG
   Serial.begin(115200);
#endif
   LOGLN("[MAIN] Starting esp32 setup");

   setup_wifi();
   led_setup();
   setup_mdns();
   setup_server();

   LOGLN("[MAIN] Setup done");
}

void loop()
{
   wifi_loop();
   led_loop();

#ifdef DEBUG
   EVERY_N_SECONDS(5) {
      auto freeBytes = xPortGetFreeHeapSize();
      LOGF("[MEM] Free memory: %d kB\n", freeBytes / 1024);
   }
#endif
}
