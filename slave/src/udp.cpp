#include "WiFi.h"
#include <AsyncUDP.h>

#include <pb_encode.h>
#include <pb_decode.h>
#include <ledctrl.pb.h>

#include "log.h"
#include "processor.h"

AsyncUDP udp;

bool firstUdpPacket = true;

void on_udp_packet(AsyncUDPPacket packet)
{
   if (firstUdpPacket)
   {
      firstUdpPacket = false;
      LOGF("[UDP] running UDP callback on core %d\n", xPortGetCoreID());
   }

   // LOG("[UDP] Packet Type: ");
   // LOG(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
   // LOG(", From: ");
   // LOG(packet.remoteIP());
   // LOG(":");
   // LOG(packet.remotePort());
   // LOG(", To: ");
   // LOG(packet.localIP());
   // LOG(":");
   // LOG(packet.localPort());
   // LOG(", Length: ");
   // LOG(packet.length());
   // LOGLN();

   ledctrl_Response response = ledctrl_Response_init_default;
   process_message(packet.data(), packet.length(), response);

   //reply to the client
   pb_byte_t buf[128] = {0};
   auto ostream = pb_ostream_from_buffer(buf, sizeof(buf));
   if (!pb_encode(&ostream, ledctrl_Response_fields, &response))
   {
      LOGF("[UDP] Could not encode response for request %d\n", response.id);
      return;
   }

   size_t buflen;
   if (!pb_get_encoded_size(&buflen, ledctrl_Response_fields, &response))
   {
      LOGF("[UDP] Could not compute reponse size for request %d\n", response.id);
      return;
   }

   packet.write(buf, buflen);
}

void setup_udp()
{
   LOGLN("[UDP] Setting up udp listener")

   if (udp.listen(UDP_PORT))
   {
      LOGLN("[UDP] Listening for udp connection")
      udp.onPacket(on_udp_packet);
   }
}