#pragma once

#include "stopwatch.h"

class WiFiUDP;

typedef void (*UdpCallback)(const uint8_t *data, int length, WiFiUDP* svr);

class Udp {
public:
   Udp(const char* id, uint16_t port, UdpCallback cb);
   void start();
   void tick();

private:
   WiFiUDP* udp;
   uint8_t* buffer;
   UdpCallback cb;
   StopWatch sw;
   uint16_t port;
   bool firsttick;
   const char* id;
};