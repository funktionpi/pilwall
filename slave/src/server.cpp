#include <ESPAsyncWebServer.h>
// #include <internal/RgbColor.h>

#include <pb_encode.h>
#include <pb_decode.h>
#include <ledctrl.pb.h>
#include <led.h>
#include "log.h"

#include "config.h"

AsyncWebServer server(CTRL_PORT);
AsyncEventSource events("/events");
AsyncWebSocket ws("/ws");

void ws_process_request(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void ws_process_message(uint8_t *data, size_t len, ledctrl_Response &response);

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
   if (type == WS_EVT_CONNECT)
   {
      LOGF("[SVR] ws[%s][%u] connect\n", server->url(), client->id());
      client->printf("You are client #%u", client->id());
      client->ping();
   }
   else if (type == WS_EVT_DISCONNECT)
   {
      LOGF("[SVR] ws[%s][%u] disconnect:\n", server->url(), client->id());
      LEDs().Clear(0);
   }
   else if (type == WS_EVT_ERROR)
   {
      LOGF("[SVR] ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
   }
   else if (type == WS_EVT_PONG)
   {
      LOGF("[SVR] ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
   }
   else if (type == WS_EVT_DATA)
   {
      ws_process_request(server, client, type, arg, data, len);
   }
}

void ws_process_request(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
   AwsFrameInfo *info = (AwsFrameInfo *)arg;
   String msg = "";
   if (info->final && info->index == 0 && info->len == len)
   {
      //the whole message is in a single frame and we got all of it's data
      LOGF("[SVR] ws[%s][%u] %s-message[%llu] \n", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

      ledctrl_Response response = ledctrl_Response_init_default;

      if (info->opcode == WS_TEXT)
      {
         for (size_t i = 0; i < info->len; i++)
         {
            msg += (char)data[i];
         }
         LOGF("[SVR] Text message: %s\n", msg.c_str());
      }
      else
      {
         ws_process_message(data, info->len, response);
      }

      if (info->opcode == WS_TEXT)
      {
         client->text("I got your text message");
      }
      else
      {
         pb_byte_t buf[64] = {0};
         auto ostream = pb_ostream_from_buffer(buf, sizeof(buf));
         if (!pb_encode(&ostream, ledctrl_Response_fields, &response))
         {
            LOGF("[SVR] Could not encode response for request %d\n", response.id);
            return;
         }
         size_t buflen;
         if (!pb_get_encoded_size(&buflen, ledctrl_Response_fields, &response))
         {
            LOGF("[SVR] Could not compute reponse size for request %d\n", response.id);
            return;
         }
         LOGF("[SVR] Sending response for request %d, size %d\n", response.id, buflen);
         client->binary((char *)buf, buflen);
      }
   }
   else
   {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if (info->index == 0)
      {
         if (info->num == 0)
         {
            LOGF("[SVR] ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
         }
         LOGF("[SVR] ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      LOGF("[SVR] ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

      if (info->opcode == WS_TEXT)
      {
         for (size_t i = 0; i < len; i++)
         {
            msg += (char)data[i];
         }
      }
      else
      {
         char buff[3];
         for (size_t i = 0; i < len; i++)
         {
            sprintf(buff, "%02x ", (uint8_t)data[i]);
            msg += buff;
         }
      }
      LOGF("[SVR] %s\n", msg.c_str());

      if ((info->index + len) == info->len)
      {
         LOGF("[SVR] ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
         if (info->final)
         {
            LOGF("[SVR] ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
            if (info->message_opcode == WS_TEXT)
               client->text("I got your text message");
            // else
            // client->binary("I got your binary message");
         }
      }
   }
}

CRGB fromProtoColor(uint32_t color)
{
   auto r = (color & 0x00ff0000) >> 16;
   auto g = (color & 0x0000ff00) >> 8;
   auto b = (color & 0x000000ff);
   // return RgbColor(r / 255.f * SATURATION, g / 255.f * SATURATION, b / 255.f * SATURATION);
   return CRGB(r, g, b);
}

void ws_process_message(uint8_t *data, size_t len, ledctrl_Response &response)
{
   LOGLN("[PB] decoding protobuf message");

   auto stream = pb_istream_from_buffer(data, len);

   ledctrl_Request msg = ledctrl_Request_init_default;

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
      LEDs().Clear(color);
      break;
   }
   case ledctrl_Request_matrix_tag:
   {
      LOGLN("[PB] received a matrix request");

      auto width = LEDs().Width();
      auto height = LEDs().Height();

      for (int x = 0; x < width; ++x)
      {
         for (int y = 0; y < height; ++y)
         {
            auto color = msg.request.matrix.pixels[x * width + y];
            LEDs().SetPixel(x, y, color);
         }
      }

      break;
   }
   case ledctrl_Request_pixels_tag:
   {
      LOGLN("[PB] received a pixels request");

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
   case ledctrl_Request_draw_line_tag:
   {
      LOGLN("[PB] received a draw_line request");

      auto x1 = (msg.request.draw_line.start.xy & 0xFFFF0000) >> 16;
      auto y1 = (msg.request.draw_line.start.xy & 0x0000FFFF);

      auto x2 = (msg.request.draw_line.end.xy & 0xFFFF0000) >> 16;
      auto y2 = (msg.request.draw_line.end.xy & 0x0000FFFF);

      auto color =  (msg.request.draw_line.color);

      LEDs().DrawLine(x1, y1, x2, y2, color);
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
   }
}

void setup_server()
{
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Hello, world");
   });

   ws.onEvent(onWsEvent);
   server.addHandler(&ws);
   server.begin();
}