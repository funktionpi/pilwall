#include <Arduino.h>
#include <AsyncUDP.h>

#include "led_controller.h"

#include <pb_encode.h>
#include <pb_decode.h>
#include <ledctrl.pb.h>

#include "log.h"
#include "cmd_processor.h"

#ifndef DEBUG_RAW
   #undef DLOG
   #undef DLOGLN
   #undef DLOGF
   #define DLOG(...)
   #define DLOGLN(...)
   #define DLOGF(...)
#endif

AsyncUDP udp_raw;

bool firstRawPacket = true;

class udp_packet_data {
public:
   uint16_t index;
   uint8_t count; // number of LEDs (not the size of the buffer)
   uint8_t *colors;
};

void on_raw_packet(AsyncUDPPacket packet)
{
   if (firstRawPacket)
   {
      firstRawPacket = false;
      LOGF("[RAW] running UDP callback on core %d\n", xPortGetCoreID());
   }

   auto packet_st_size = sizeof(udp_packet_data);
   if (packet.length() < packet_st_size)
   {
      LOGF("[RAW] invalid packet size received, expecting at least %d, received %d\n", packet_st_size, packet.length());
   }

   auto data = (udp_packet_data *)packet.data();

   auto packet_len = data->count * sizeof(CRGB) + sizeof(uint16_t) + sizeof(uint8_t);
   if (packet_len > packet.length())
   {
      LOGF("[RAW] packet data has wrong length, expecting at least %d, received %d", packet_len, packet.length())
      return;
   }

   if ( data->count > 64 )
   {
      LOGF("[RAW] data packet is too large, truncating data");
      data->count = 64;
   }

   LEDs().Lock();
   LEDs().CopyRaw(data->index, (const char *) data->colors, data->count);
   LEDs().Unlock();
}

void setup_raw()
{
   LOGF("[RAW] Setting up raw listener on port %d\n", RAW_PORT)

   if (udp_raw.listen(RAW_PORT))
   {
      LOGLN("[RAW] raw socket open")
      udp_raw.onPacket(on_raw_packet);
   }
}