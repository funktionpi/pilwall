#include <pb_encode.h>
#include <pb_decode.h>

#include "log.h"

#include "led_controller.h"
#include "cmd_processor.h"

#if !DEBUG_CMD
#undef DLOG
#undef DLOGLN
#undef DLOGF
#define DLOG(...)
#define DLOGLN(...)
#define DLOGF(...)
#endif

CRGB fromProtoColor(uint32_t color)
{
   auto r = (color & 0x00ff0000) >> 16;
   auto g = (color & 0x0000ff00) >> 8;
   auto b = (color & 0x000000ff);
   return CRGB(r, g, b);
}

void process_message(const uint8_t *data, size_t len, piproto_Response &response)
{
   piproto_Request msg = piproto_Request_init_default;

   // LOGLN("[PB] reading from buffer message");
   auto stream = pb_istream_from_buffer(data, len);

   // LOGLN("[PB] decoding protobuf message");
   auto status = pb_decode(&stream, piproto_Request_fields, &msg);

   if (!status)
   {
      DLOGF("[PB] Decoding failed: %s\n", PB_GET_ERROR(&stream));
      return;
   }

   response.id = msg.id;
   DLOGF("[PB] Id: %d, Tag: %d\n", msg.id, msg.which_request);

   switch (msg.which_request)
   {
   case piproto_Request_dimension_tag:
   {
      LOGLN("[PB] received a dimension request");
      response.which_response = piproto_Response_dimension_tag;
      response.response.dimension.width = LEDs().Width();
      response.response.dimension.height = LEDs().Height();
      break;
   }
   case piproto_Request_raw_tag:
   {
      auto count = msg.request.raw.pixels_count * 4 / 3;
      DLOGF("[PB] received a raw pixel request, index: %d, count: %d\n", msg.request.raw.index, count);
      LEDs().CopyRaw(msg.request.raw.index, (uint8_t *)msg.request.raw.pixels, count);
      break;
   }
   case piproto_Request_clear_tag:
   {
      DLOGLN("[PB] received a clear request");
      auto color = CRGB(msg.request.clear.color);
      DLOGF("[PB] Clear color to (r: %d, g: %d, b: %d)\n", color.red, color.green, color.blue);

      LEDs().Clear(color);
      break;
   }
   case piproto_Request_pixels_tag:
   {
      DLOGF("[PB] received with %d pixels request\n", msg.request.pixels.pixels_count);
      for (int it = 0; it < msg.request.pixels.pixels_count; ++it)
      {
         auto pixel = msg.request.pixels.pixels[it];
         auto x = (pixel.coord.xy & 0xFFFF0000) >> 16;
         auto y = (pixel.coord.xy & 0x0000FFFF);
         auto color = fromProtoColor(pixel.color);
         LEDs().SetPixel(x, y, color);
      }
      break;
   }
   case piproto_Request_draw_line_tag:
   {
      DLOGLN("[PB] received a draw_line request");
      auto x1 = (msg.request.draw_line.start.xy & 0xFFFF0000) >> 16;
      auto y1 = (msg.request.draw_line.start.xy & 0x0000FFFF);

      auto x2 = (msg.request.draw_line.end.xy & 0xFFFF0000) >> 16;
      auto y2 = (msg.request.draw_line.end.xy & 0x0000FFFF);

      auto color = (msg.request.draw_line.color);

      LEDs().DrawLine(x1, y1, x2, y2, color);
      break;
   }
   case piproto_Request_brightness_tag:
   {
      DLOGLN("[PB] received a set brightness request");

      auto val = msg.request.brightness.brightness;
      if (val > 255)
      {
         val = 255;
      }
      else if (val < 0)
      {
         val = 0;
      }

      LEDs().SetBrightness(val);
      break;
   }
   case piproto_Request_update_tag:
   {
      DLOGLN("[PB] received an update request");
      LEDs().Update();
      break;
   }
   }
}

size_t encode_response(const piproto_Response &response, uint8_t *outbuffer, int bufalloc)
{
   auto ostream = pb_ostream_from_buffer((pb_byte_t *)outbuffer, bufalloc);
   if (!pb_encode(&ostream, piproto_Response_fields, &response))
   {
      DLOGF("[PB] Could not encode response for request %d\n", response.id);
      return 0;
   }

   size_t outlen;
   if (!pb_get_encoded_size(&outlen, piproto_Response_fields, &response))
   {
      DLOGF("[PB] Could not compute reponse size for request %d\n", response.id);
   }
   return outlen;
}