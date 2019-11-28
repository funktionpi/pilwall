#include "config.h"
#include "main.h"
#include "led.h"
#include "log.h"

void setup()
{
#ifdef SERIAL_DEBUG
   Serial.begin(115200);
#endif
   LOGLN("[MAIN] Starting esp32 setup");

   LOGF("[MAIN] running setup on core %d\n", xPortGetCoreID());

   setup_wifi();
   setup_led();
   setup_mdns();
   setup_udp();
   setup_raw();
   setup_artnet();

   // setup_websocket();
   // setup_tcp();

   LOGLN("[MAIN] Setup done");
}

bool mainFirstTick = true;

void loop()
{
   if (mainFirstTick)
   {
      mainFirstTick = false;
      LOGF("[MAIN] running tick on core %d\n", xPortGetCoreID());
      auto freeBytes = xPortGetFreeHeapSize();
      LOGF("[MEM] Free memory: %d kB\n", freeBytes / 1024);
   }

   wifi_tick();
   artnet_tick();
   led_tick();

}