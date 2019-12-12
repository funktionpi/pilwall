
#include <Arduino.h>
#include <WiFiUdp.h>
#include <FastLED.h>

#include "config.h"
#include "log.h"
#include "Udp.h"

Udp::Udp(const char* id, uint16_t port, UdpCallback cb)
:  cb(cb),
   port(port),
   firsttick(true),
   id(id)
{
   buffer = new uint8_t[MAX_UDP_PACKET_SIZE];
   udp = new WiFiUDP();
   sw.reset();
}

void Udp::start()
{
   if (udp->begin(port))
   {
      LOGF("[%s] listening to port udp %d\n", id, port)
   }
}

void Udp::tick()
{
   if (firsttick)
   {
      firsttick = false;
      LOGF("[%s] running udp tick on core %d\n", id, xPortGetCoreID());
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
         LOGF("[%s] took average of %d ms to process %d udp packets\n", id, sw.averagems(), sw.runs());
         sw.reset();
      }
   }
}