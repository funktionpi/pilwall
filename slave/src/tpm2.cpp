#include "config.h"

#if ENABLE_TPM2

#include <WiFiUdp.h>

#include "led_controller.h"
#include "stopwatch.h"

#include "log.h"
#include "udp.h"

#if !DEBUG_TPM2
#undef DLOG
#undef DLOGLN
#undef DLOGF
#define DLOG(...)
#define DLOGLN(...)
#define DLOGF(...)
#endif

typedef struct
{
   uint8_t start;
   uint8_t type;

   uint8_t frameSizeH;
   uint8_t frameSizeL;

   uint8_t packet_index;
   uint8_t packet_count;

   uint8_t data[]; // up to 1,490 bytes

} tpm2_packet;

const uint8_t packet_start_byte = 0x9c;
const uint8_t packet_type_data = 0xda;
const uint8_t packet_type_cmd = 0xc0;
const uint8_t packet_type_response = 0xaa;
const uint8_t packet_end_byte = 0x36;
const uint8_t packet_response_ack = 0xac;

void on_tpm2_packet(const uint8_t *data, int length, WiFiUDP *svr)
{
   if (length < sizeof(tpm2_packet))
   {
      DLOGF("[TPM2] packet size (%dB)is too small to be a tpm2 packet (%dB)\n", length, sizeof(tpm2_packet));
   }

   auto packet = (tpm2_packet *)data;

   // check for proper header
   if (packet->start != packet_start_byte)
   {
      DLOGLN("[TPM2] header doesn't start with proper 0x9C header");
      return;
   }

   uint16_t frame_size = (packet->frameSizeH << 8) | packet->frameSizeL;

   DLOGF("[TPM2] received packet, type: 0x%x, count: %d, index: %d, udp packet size: %d, frame_size: %d\n",
         packet->type, packet->packet_count, packet->packet_index, length, frame_size);

   if (packet->data[frame_size] != packet_end_byte)
   {
      DLOGF("[TPM2] invalid packet ending: 0x%x\n", packet->data[frame_size]);
      return;
   }

   switch (packet->type)
   {
   case packet_type_cmd:
   {
      auto color = CRGB(packet->data[1], packet->data[2], packet->data[3]);
      DLOGF("[TPM2] clear color command: (r: %d, g: %d, b: %d)\n", color.r, color.g, color.b);
      LEDs().Clear(color);
      break;
   }

   case packet_type_data:
   {
      auto ledCount = frame_size / sizeof(CRGB);
      auto ledPerPacket = MATRIX_SIZE / packet->packet_count;
      auto index = ledPerPacket * (packet->packet_index - 1); // packet are one based

      DLOGF("[TPM2] received data for pixels %d to %d\n", index, index + ledCount);
      LEDs().CopyRaw(index, packet->data, ledCount);
      LEDs().Update();
      break;
   }

   case packet_type_response:
   {
      DLOGLN("[TPM2] received response packet");
      svr->beginPacket(svr->remoteIP(), TPM2_OUT_PORT);
      svr->write(&packet_response_ack, 1);
      svr->endPacket();

      LEDs().Update();
      break;
   }

   default:
   {
      DLOGF("[TPM2] unknown packet type: 0x%x\n", packet->type)
      break;
   }
   }
}

Udp tpm2_udp("TPM2", TPM2_IN_PORT, on_tpm2_packet);

void setup_tpm2()
{
   LOGLN("[TPM2] setting up tpm2.net receiver");
   tpm2_udp.start();
}

void tick_tpm2()
{
   tpm2_udp.tick();
}

#endif