#include <Arduino.h>

#include <pb_encode.h>
#include <pb_decode.h>
#include <ledctrl.pb.h>

#include "log.h"

#ifndef DEBUG_TCP
   #undef DLOG
   #undef DLOGLN
   #undef DLOGF
   #define DLOG(...)
   #define DLOGLN(...)
   #define DLOGF(...)
#endif

#include "AsyncTCP.h"

#include "cmd_processor.h"

AsyncServer tcp_server(TCP_PORT);

// static std::vector<AsyncClient *> tcp_clients;

/* tcp_clients events */
static void handleError(void *arg, AsyncClient *client, int8_t error)
{
   LOGF("[TCP] connection error %s from client %s\n", client->errorToString(error), client->remoteIP().toString().c_str());
}

static void handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
   DLOGF("[TCP] data received from client %s, size: %d\n", client->remoteIP().toString().c_str(), len);

   ledctrl_Response response;
   process_message((uint8_t*)data, len, response);

   uint8_t buf[128] = {0};
   auto outlen = encode_response(response, buf, sizeof(buf));

   // reply to client
   if (client->space() > outlen && client->canSend())
   {
      client->add((char*)buf, outlen);
      client->send();
   }
}

static void handleDisconnect(void *arg, AsyncClient *client)
{
   LOGF("[TCP] client %s disconnected \n", client->remoteIP().toString().c_str());
}

static void handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
   LOGF("[TCP] client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
}

bool firstTcpPacket = true;

/* server events */
static void handleNewClient(void *arg, AsyncClient *client)
{
   if (firstTcpPacket)
   {
      firstTcpPacket = false;
      LOGF("[TCP] running TCP callback on core %d\n", xPortGetCoreID());
   }

   LOGF("[TCP] new client has been connected to server, ip: %s", client->remoteIP().toString().c_str());

   // add to list
   // tcp_clients.push_back(client);

   // register events
   client->onData(&handleData, NULL);
   client->onError(&handleError, NULL);
   client->onDisconnect(&handleDisconnect, NULL);
   client->onTimeout(&handleTimeOut, NULL);
}

void setup_tcp()
{
   tcp_server.onClient(&handleNewClient, &tcp_server);
   LOGF("[TCP] Setting up tcp server in port %d\n", TCP_PORT)
   tcp_server.begin();
}