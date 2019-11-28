#include <Arduino.h>
#include <AsyncUDP.h>

#include "led_controller.h"

#include <pb_encode.h>
#include <pb_decode.h>
#include <ledctrl.pb.h>

#include "log.h"
#include "cmd_processor.h"

#ifndef DEBUG_UDP
   #undef DLOG
   #undef DLOGLN
   #undef DLOGF
   #define DLOG(...)
   #define DLOGLN(...)
   #define DLOGF(...)
#endif

AsyncUDP udp_cmd;

bool firstUdpPacket = true;

class udp_packet_data {
public:
   uint16_t index;
   uint8_t count; // number of LEDs (not the size of the buffer)
   uint8_t *colors;
};

void on_udp_packet(AsyncUDPPacket packet)
{
   if (firstUdpPacket)
   {
      firstUdpPacket = false;
      LOGF("[UDP] running UDP callback on core %d\n", xPortGetCoreID());

      if (xPortInIsrContext())
      {
         LOGLN("[UDP] executing from interupt context")
      }
   }

   DLOGF("[UDP] received UDP packet, size: %d\n", packet.length());

   ledctrl_Response response;
   process_message((uint8_t*)packet.data(), packet.length(), response);

   uint8_t buf[128] = {0};
   auto outlen = encode_response(response, buf, sizeof(buf));

   if (outlen > 0)
   {
      packet.write(buf, outlen);
   }
}

void setup_udp()
{
   LOGF("[UDP] Setting up udp listener on port %d\n", UDP_PORT)

   if (udp_cmd.listen(UDP_PORT))
   {
      LOGLN("[UDP] raw socket open")
      udp_cmd.onPacket(on_udp_packet);
   }
}