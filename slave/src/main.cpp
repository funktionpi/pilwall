#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>

#include "config.h"
#include "ledslave.h"
#include "led.h"

void setup()
{
   Serial.begin(115200);
   Serial.println("Starting esp32 setup");

   setup_wifi();
   led_setup();
   setup_mdns();

   Serial.println("Setup done");
}

void loop()
{
   wifi_loop();
   led_loop();
}
