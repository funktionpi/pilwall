#include <ESPAsyncWebServer.h>
// #include <internal/RgbColor.h>

#include <pb_encode.h>
#include <pb_decode.h>
#include <ledctrl.pb.h>
#include <led.h>

#include "config.h"
#include "processor.h"

// #define DEBUG_NETWORK

#ifdef DEBUG_NETWORK
  #define LOG(...) Serial.print(__VA_ARGS__);
  #define LOGLN(...) Serial.println(__VA_ARGS__);
  #define LOGF(...) Serial.printf(__VA_ARGS__);
#else
  #define LOG(...)
  #define LOGLN(...)
  #define LOGF(...)
#endif

AsyncWebServer server(CTRL_PORT);
AsyncEventSource events("/events");
AsyncWebSocket ws("/ws");

void ws_process_request(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void ws_process_message(uint8_t *data, size_t len, ledctrl_Response &response);

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
   if (type == WS_EVT_CONNECT)
   {
      Serial.printf("[SVR] running events on core %d\n", xPortGetCoreID());
      Serial.printf("[SVR] ws[%s][%u] connect\n", server->url(), client->id());

      client->printf("You are client #%u", client->id());
      client->ping();
   }
   else if (type == WS_EVT_DISCONNECT)
   {
      Serial.printf("[SVR] ws[%s][%u] disconnect:\n", server->url(), client->id());
      LEDs().Clear(0);
      LEDs().Update();
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
         process_message(data, info->len, response);
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

void setup_websocket()
{
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Hello, world");
   });

   ws.onEvent(onWsEvent);
   server.addHandler(&ws);
   server.begin();
}