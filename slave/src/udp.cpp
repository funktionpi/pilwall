#include <Arduino.h>
#include <WiFiUdp.h>
#include <FastLED.h>

#include "config.h"
#include "log.h"
#include "Udp.h"

Udp::Udp(uint16_t port, UdpCallback cb)
:  cb(cb),
   port(port),
   firsttick(true)
{
   buffer = new uint8_t[MAX_UDP_PACKET_SIZE];
   udp = new WiFiUDP();
   sw.reset();
}

void Udp::start()
{
   if (udp->begin(port))
   {
      LOGF("[UDP] listening to port udp %d\n", port)
   }
}

void Udp::tick()
{
   if (firsttick)
   {
      firsttick = false;
      LOGF("[UDP] running tick on core %d\n", xPortGetCoreID());
   }

   if (udp->parsePacket())
   {
      sw.start();
      while (udp->available())
      {
         auto packetSize = udp->read((unsigned char*)buffer, MAX_UDP_PACKET_SIZE);
         if (packetSize)
         {
            cb(buffer, packetSize, udp);
         }
      }
      sw.stop();
   }

   EVERY_N_SECONDS(5)
   {
      if (sw.runs())
      {
         LOGF("[UDP] took average of %d ms to process %d udp commands\n", sw.averagems(), sw.runs());
         sw.reset();
      }
   }
}