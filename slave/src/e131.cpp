#include "config.h"

#if ENABLE_E131

#include <ESPAsyncE131.h>

#include "log.h"
#include "led_controller.h"
#include "stopwatch.h"

#if !DEBUG_E131
#undef DLOG
#undef DLOGLN
#undef DLOGF
#define DLOG(...)
#define DLOGLN(...)
#define DLOGF(...)
#endif

static const int startUniverse = 1;
static const int ledsPerUniverse = 128;
static const int channelsPerUniverse = ledsPerUniverse * 3;
static const int maxUniverses = MATRIX_SIZE / ledsPerUniverse;

static ESPAsyncE131 e131(maxUniverses);
static StopWatch sw;

void setup_e131()
{
   LOGLN("[E131] setting up ACN e131 controller");

   if (e131.begin(E131_UNICAST))
   {
      LOGF("[E131] listening for ACN e131 on port %d\n", E131_DEFAULT_PORT);
   }
   sw.reset();
}

void tick_e131()
{
   while (!e131.isEmpty())
   {
      sw.start();
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
      LEDs().CopyRaw(index, packet.property_values, ledCount);
      LEDs().Update();

      sw.stop();
   }

   EVERY_N_SECONDS(5)
   {
      if (sw.runs())
      {
         LOGF("[e131] took average of %d us to process %d udp packets\n", sw.average(), sw.runs());
         sw.reset();
      }
   }
}

#endif