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

#if ENABLE_ARTNET
   setup_artnet();
#endif

#if ENABLE_OPC
   setup_opc();
#endif

#if ENABLE_PROTO
   setup_proto();
#endif

// setup_raw();
#if ENABLE_E131
   setup_e131();
#endif

#if ENABLE_TPM2
   setup_tpm2();
#endif

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

   tick_wifi();
   tick_led();

#if ENABLE_ARTNET
   tick_artnet();
#endif

#if ENABLE_TPM2
   tick_tpm2();
#endif

#if ENABLE_E131
   tick_e131();
#endif

#if ENABLE_PROTO
   tick_proto();
#endif

#if ENABLE_OPC
   tick_opc();
#endif

}