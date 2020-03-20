#include "config.h"

#if ENABLE_ARTNET

#include <Artnet.h>

#include "led_controller.h"
#include "log.h"
#include "led.h"
#include "stopwatch.h"

#if !DEBUG_ARTNET
#undef DLOG
#undef DLOGLN
#undef DLOGF
#define DLOG(...)
#define DLOGLN(...)
#define DLOGF(...)
#endif

const int startUniverse = 1;
const int ledsPerUniverse = 128;
const int channelsPerUniverse = ledsPerUniverse * 3;
const int maxUniverses = MATRIX_SIZE / ledsPerUniverse;

ArtnetReceiver artnet;
StopWatch artnet_sw(false);

void onDmxFrame(uint8_t universe, uint8_t *data, uint16_t length)
{
   artnet_sw.start();

   DLOGF("[ARTNET] received dmx frame, universe: %d, length: %d\n", universe, length);

   if (universe < startUniverse || universe > maxUniverses)
   {
      LOGF("[ARTNET] invalid universe: %d, only support range [%d-%d]", universe, startUniverse, maxUniverses);
   }

   // read universe and put into the right part of the display buffer
   int index = (universe - startUniverse) * ledsPerUniverse;
   int ledCount = length / 3;
   if (ledCount > ledsPerUniverse)
   {
      // DLOGLN("[ARTNET] data overflow");
      ledCount = ledsPerUniverse;
   }
   DLOGF("[ARTNET] writing to index %d, led count : %d\n", index, ledCount);
   LEDs().copyRaw(index, data, ledCount);
   LEDs().update();

   artnet_sw.stop();
}

void setup_artnet()
{
   LOGF("[ARTNET] Setting up Artnet controller\n");

   for (int i = startUniverse; i <= maxUniverses; ++i)
   {
      auto idx = (i - startUniverse) * ledsPerUniverse;
      LOGF("[ARTNET] registering universe #%d for led [%d-%d]\n", i, idx, idx + ledsPerUniverse - 1);
      artnet.subscribe(i, [i](uint8_t *data, uint16_t size) {
         onDmxFrame(i, data, size);
      });
   }

   artnet.begin(ARTNET_PORT);
   LOGF("[ARTNET] Artnet setup done.\n");
}

void tick_artnet()
{
   artnet.parse();

   EVERY_N_SECONDS(5)
   {
      if (artnet_sw.runs())
      {
         LOGF("[ARTNET] took average of %d us to process %d packets\n", artnet_sw.average(), artnet_sw.runs());
         artnet_sw.reset();
      }
   }
}

#endif