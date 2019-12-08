
#include <ESPAsyncE131.h>

#include "config.h"
#include "log.h"
#include "led_controller.h"

#ifndef DEBUG_E131
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

ESPAsyncE131 e131(maxUniverses);

void setup_e131()
{
   LOGLN("[E131] setting up ACN e131 controller");

   if (e131.begin(E131_UNICAST))
   {
      LOGF("[E131] listening for ACN e131 on port %d\n", E131_DEFAULT_PORT);
   }

   //   if (e131.begin(E131_MULTICAST, UNIVERSE, UNIVERSE_COUNT))   // Listen via Multicast
   //    {
   //       Serial.println(F("Listening for data..."));
   //    }
   //    else
   //    {
   //       Serial.println(F("*** e131.begin failed ***"));
   //    }
}

void tick_e131()
{
   if (!e131.isEmpty())
   {
      e131_packet_t packet;
      e131.pull(&packet); // Pull packet from ring buffer

      auto universe = htons(packet.universe);
      auto data_length = htons(packet.property_value_count) - 1;

      DLOGF("[e131] Universe %u / %u Channels | Packet#: %u / Errors: %u\n",
            htons(packet.universe),                 // The Universe for this packet
            htons(packet.property_value_count) - 1, // Start code is ignored, we're interested in dimmer data
            e131.stats.num_packets,                 // Packet counter
            e131.stats.packet_errors);              // Packet error counter

      if (universe < startUniverse || universe > maxUniverses)
      {
         DLOGF("[e131] invalid universe: %d\n", universe)
         return;
      }

      auto ledCount = data_length / 3;
      auto index = (universe - startUniverse) * ledsPerUniverse;
      DLOGF("[e131] received data for pixels %d to %d\n", index, index + ledCount);
      LEDs().Lock();
      LEDs().CopyRaw(index, packet.property_values, ledCount);
      LEDs().Unlock();
      LEDs().Update();
   }
}