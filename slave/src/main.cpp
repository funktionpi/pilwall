#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>

#include "config.h"
#include "ledslave.h"
#include "led.h"

#define DEBUG

void setup()
{
#ifdef DEBUG
   Serial.begin(115200);
#endif
   Serial.println("[MAIN] Starting esp32 setup");

   setup_wifi();
   led_setup();
   setup_mdns();
   setup_server();

   Serial.println("[MAIN] Setup done");
}

void loop()
{
   wifi_loop();
   led_loop();
}
