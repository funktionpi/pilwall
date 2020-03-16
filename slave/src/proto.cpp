#include "config.h"

#if ENABLE_PROTO

#include <Arduino.h>
#include <WiFiUdp.h>

#include "led_controller.h"
#include "stopwatch.h"
#include "udp.h"

#include <pb_encode.h>
#include <pb_decode.h>
#include <pi.pb.h>

#include "log.h"
#include "cmd_processor.h"

#if !DEBUG_PROTO
   #undef DLOG
   #undef DLOGLN
   #undef DLOGF
   #define DLOG(...)
   #define DLOGLN(...)
   #define DLOGF(...)
#endif

class udp_packet_data {
public:
   uint16_t index;
   uint8_t count; // number of LEDs (not the size of the buffer)
   uint8_t *colors;
};

void on_proto_packet(const uint8_t* data, int length, WiFiUDP* svr)
{
   DLOGF("[PROTO] received UDP packet, size: %d\n", length);

   piproto_Response response;
   process_message(data, length, response);

   uint8_t buf[128] = {0};
   auto outlen = encode_response(response, buf, sizeof(buf));

   if (outlen > 0)
   {
      if (!svr->beginPacket(svr->remoteIP(), svr->remotePort()))
      {
         LOGLN("[PROTO] could not write response packet");
      }

      svr->write(buf, outlen);

      if (svr->endPacket() < 0)
      {
         LOGLN("[PROTO] failed to send response packet");
      }
      // udp_wifi.flush();
   }
}

Udp proto_udp("PROTO", PROTO_PORT, on_proto_packet);

void setup_proto()
{
   LOGLN("[PROTO] setting up proto receiver");
   proto_udp.start();
}

void tick_proto()
{
   proto_udp.tick();
}

#endif