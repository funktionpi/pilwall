#include <pb_encode.h>
#include <pb_decode.h>

#include "config.h"
#include "log.h"

#include "led.h"
#include "processor.h"

// #define DEBUG_PROTO
#ifndef DEBUG_PROTO
#undef LOGLN
#define LOGLN
#undef LOGF
#define LOGF
#endif

CRGB fromProtoColor(uint32_t color)
{
   auto r = (color & 0x00ff0000) >> 16;
   auto g = (color & 0x0000ff00) >> 8;
   auto b = (color & 0x000000ff);
   // return RgbColor(r / 255.f * SATURATION, g / 255.f * SATURATION, b / 255.f * SATURATION);
   return CRGB(r, g, b);
}

ledctrl_Request msg = ledctrl_Request_init_default;

void process_message(uint8_t *data, size_t len, ledctrl_Response &response)
{
   LOGLN("[PB] reading from buffer message");
   auto stream = pb_istream_from_buffer(data, len);

   LOGLN("[PB] decoding protobuf message");
   auto status = pb_decode(&stream, ledctrl_Request_fields, &msg);

   if (!status)
   {
      LOGF("[PB] Decoding failed: %s\n", PB_GET_ERROR(&stream));
      return;
   }

   response.id = msg.id;
   LOGF("[PB] Id: %d, Tag: %d\n", msg.id, msg.which_request);

   switch (msg.which_request)
   {
   case ledctrl_Request_dimension_tag:
   {
      LOGLN("[PB] received a dimension request");
      response.which_response = ledctrl_Response_dimension_tag;
      response.response.dimension.width = LEDs().Width();
      response.response.dimension.height = LEDs().Height();
      break;
   }
   case ledctrl_Request_clear_tag:
   {
      LOGLN("[PB] received a clear request");
      auto color = CRGB(msg.request.clear.color);
      LOGF("[PB] Clear color to (r: %d, g: %d, b: %d)\n",
                    color.red, color.green, color.blue);

      LEDs().Lock();
      LEDs().Clear(color);
      LEDs().Unlock();
      break;
   }
   case ledctrl_Request_matrix_tag:
   {
      LOGLN("[PB] received a matrix request");

      auto width = LEDs().Width();
      auto height = LEDs().Height();

      LEDs().Lock();
      for (int x = 0; x < width; ++x)
      {
         for (int y = 0; y < height; ++y)
         {
            auto color = msg.request.matrix.pixels[x * width + y];
            LEDs().SetPixel(x, y, color);
         }
      }
      LEDs().Unlock();
      break;
   }
   case ledctrl_Request_pixels_tag:
   {
      LOGLN("[PB] received a pixels request");
      LEDs().Lock();
      for (int it = 0; it < msg.request.pixels.pixels_count; ++it)
      {
         auto pixel = msg.request.pixels.pixels[it];
         auto x = (pixel.coord.xy & 0xFFFF0000) >> 16;
         auto y = (pixel.coord.xy & 0x0000FFFF);
         auto color = fromProtoColor(pixel.color);
         LEDs().SetPixel(x, y, color);
      }
      LEDs().Unlock();
      break;
   }
   case ledctrl_Request_draw_line_tag:
   {
      LOGLN("[PB] received a draw_line request");

      auto x1 = (msg.request.draw_line.start.xy & 0xFFFF0000) >> 16;
      auto y1 = (msg.request.draw_line.start.xy & 0x0000FFFF);

      auto x2 = (msg.request.draw_line.end.xy & 0xFFFF0000) >> 16;
      auto y2 = (msg.request.draw_line.end.xy & 0x0000FFFF);

      auto color =  (msg.request.draw_line.color);

      LEDs().Lock();
      LEDs().DrawLine(x1, y1, x2, y2, color);
      LEDs().Unlock();
      break;
   }
   case ledctrl_Request_brightness_tag:
   {
      LOGLN("[PB] received a set brightness request");

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
   case ledctrl_Request_update_tag:
   {
      LOGLN("[PB] received an update request");
      LEDs().Update();
      break;
   }
   }
}
