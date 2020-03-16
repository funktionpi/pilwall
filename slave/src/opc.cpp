#include "config.h"

#if ENABLE_OPC

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

#undef ARDUINO
#include <OpcServer.h>

#include "log.h"
#include "led_controller.h"

#if !DEBUG_OPC
   #undef DLOG
   #undef DLOGLN
   #undef DLOGF
   #define DLOG(...)
   #define DLOGLN(...)
   #define DLOGF(...)
#endif

const int OPC_MAX_CHANNEL = 128;
const int OPC_LED_PER_CHANNEL = MATRIX_SIZE / OPC_MAX_CHANNEL;
const int OPC_BUFFER_SIZE = OPC_LED_PER_CHANNEL * 3 + OPC_HEADER_BYTES;

WiFiServer* wifiServer;
OpcServer* opcServer;
OpcClient opcClients[OPC_MAX_CLIENTS];
uint8_t opcBuffer[OPC_BUFFER_SIZE];

// Callback when a full OPC Message has been received
void cbOpcMessage(uint8_t channel, uint8_t command, uint16_t length, uint8_t *data)
{
   DLOGF("[OPC] received message, channel: %d, command: %d, length: %d\n", channel, command, length);

   switch (command)
   {
   // 8 bits rgb
   case 0:
   {
      auto clength = length / 3 * 3; // only keep a length that match RGB
      auto index = channel * OPC_LED_PER_CHANNEL;
      if (index + clength > MATRIX_SIZE * sizeof(CRGB))
      {
         LOGF("[OPC] data overflow")
      }
      LEDs().CopyRaw(index, data, clength);
      break;
   }
   // 16 bits rgb
   case 2:
   {
      DLOGLN("[OPC] 16 bits colors aren't supported.");
      break;
   }
   default:
{
      DLOGF("[OPC] unsupported command type: %d,\n", command)
      break;
   }
   }
}

// Callback when a client is connected
void cbOpcClientConnected(WiFiClient &client)
{
   LOGF("[OPC] Client connected\n");
}

// Callback when a client is disconnected
void cbOpcClientDisconnected(OpcClient &client)
{

   LOGF("[OPC] Client disconnected: %s\n", client.ipAddress.toString().c_str());
}

void setup_opc()
{
   LOGF("[OPC] Setting up opc listener on port %d\n", OPC_PORT);

   wifiServer = new WiFiServer(OPC_PORT);

   opcServer = new OpcServer(*wifiServer,
                             OPC_MAX_CHANNEL,
                             opcClients,
                             OPC_MAX_CLIENTS,
                             opcBuffer,
                             OPC_BUFFER_SIZE,
                             cbOpcMessage,
                             cbOpcClientConnected,
                             cbOpcClientDisconnected);

   if (!opcServer->begin())
   {
      LOGF("[OPC] opc server started %d\n", OPC_PORT);
   }
}

void tick_opc()
{
   opcServer->process();
}

#endif