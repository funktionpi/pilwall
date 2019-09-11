#include <ESPAsyncWebServer.h>
#include <internal/RgbColor.h>

#include <pb_encode.h>
#include <pb_decode.h>
#include <ledctrl.pb.h>
#include <led.h>

#include "config.h"

AsyncWebServer server(CTRL_PORT);
AsyncEventSource events("/events");
AsyncWebSocket ws("/ws");

void ws_process_request(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void ws_process_message(uint8_t *data, size_t len);

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
   if (type == WS_EVT_CONNECT)
   {
      Serial.printf("[SVR] ws[%s][%u] connect\n", server->url(), client->id());
      client->printf("[SVR] Hello Client %u :)", client->id());
      client->ping();
   }
   else if (type == WS_EVT_DISCONNECT)
   {
      Serial.printf("[SVR] ws[%s][%u] disconnect:\n", server->url(), client->id());
   }
   else if (type == WS_EVT_ERROR)
   {
      Serial.printf("[SVR] ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
   }
   else if (type == WS_EVT_PONG)
   {
      Serial.printf("[SVR] ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
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
      Serial.printf("[SVR] ws[%s][%u] %s-message[%llu] \n", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

      if (info->opcode == WS_TEXT)
      {
         for (size_t i = 0; i < info->len; i++)
         {
            msg += (char)data[i];
         }
         Serial.printf("[SVR] Text message: %s\n", msg.c_str());
      }
      else
      {
         ws_process_message(data, info->len);
      }

      if (info->opcode == WS_TEXT)
      {
         client->text("I got your text message");
      }
      else
      {
         // Serial.println("[SVR] Sending response");
         // ledctrl_EmptyResponse resp = ledctrl_EmptyResponse_init_default;
         // pb_byte_t buf[64] = {0};
         // auto ostream = pb_ostream_from_buffer(buf, sizeof(buf));
         // pb_encode(&ostream, ledctrl_EmptyResponse_fields, &resp);
         // client->binary((char *)buf);
      }
   }
   else
   {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if (info->index == 0)
      {
         if (info->num == 0)
         {
            Serial.printf("[SVR] ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
         }
         Serial.printf("[SVR] ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("[SVR] ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

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
      Serial.printf("[SVR] %s\n", msg.c_str());

      if ((info->index + len) == info->len)
      {
         Serial.printf("[SVR] ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
         if (info->final)
         {
            Serial.printf("[SVR] ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
            if (info->message_opcode == WS_TEXT)
               client->text("I got your text message");
            else
               client->binary("I got your binary message");
         }
      }
   }
}

void ws_process_message(uint8_t *data, size_t len)
{
   Serial.println("[PB] decoding protobuf message");

   auto stream = pb_istream_from_buffer(data, len);

   ledctrl_Request msg = ledctrl_Request_init_default;

   auto status = pb_decode(&stream, ledctrl_Request_fields, &msg);

   if (!status)
   {
      Serial.printf("[PB] Decoding failed: %s\n", PB_GET_ERROR(&stream));
      return;
   }

   Serial.printf("[PB] Tag: %d\n", msg.which_request);

   switch (msg.which_request)
   {
   case ledctrl_Request_clear_request_tag:
   {
      Serial.println("[PB] received a clear request");
      auto color = msg.request.clear_request.color;

      auto r = (color & 0x00ff0000) >> 16;
      auto g = (color & 0x0000ff00) >> 8;
      auto b = (color & 0x000000ff);
      RgbColor rgb(r/255.f*SATURATION,g/255.f*SATURATION,b/255.f*SATURATION);

      Serial.printf("[PB] Clear color to from (r: %d, g: %d, b: %d) to (r: %d, g: %d, b: %d)\n", r, g, b, rgb.R, rgb.G, rgb.B);

      led_clear(rgb);
      break;
   }
   case ledctrl_Request_display_image_request_tag:
   {
      Serial.println("[PB] received a display image request");
      break;
   }
   case ledctrl_Request_brightness_request_tag:
   {
      Serial.println("[PB] received a set brightness request");

      auto val = msg.request.brightness_request.brightness;
      if (val > 255)
      {
         val = 255;
      }
      else if (val < 0)
      {
         val = 0;
      }

      led_set_brightness(val);
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