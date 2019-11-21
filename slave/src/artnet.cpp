
#include <ArtnetWifi.h>
#include <WiFiUdp.h>

#include "log.h"
#include "config.h"
#include "led.h"

ArtnetWifi artnet;
const int startUniverse = 1;

const int numberOfChannels = MATRIX_SIZE * 3;

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
   // LOGF("[DMX] received dmx frame\n");

   sendFrame = 1;
   // set brightness of the whole strip
   if (universe == 15)
   {
      LEDs().SetBrightness(data[0]);
   }

   // Store which universe has got in
   if ((universe - startUniverse) < maxUniverses)
   {
      universesReceived[universe - startUniverse] = 1;
   }

   for (int i = 0; i < maxUniverses; i++)
   {
      if (universesReceived[i] == 0)
      {
         // LOGLN("[DMX] broke");
         sendFrame = 0;
         break;
      }
   }

   // read universe and put into the right part of the display buffer
   LEDs().Lock();
   for (int i = 0; i < length / 3; i++)
   {
      int led = i + (universe - startUniverse) * (previousDataLength / 3);
      if (led < MATRIX_SIZE) {
         uint16_t x = led % MATRIX_WIDTH;
         uint16_t y = led / MATRIX_WIDTH;
         // LOGF("[DMX] setting led #%d = (%d,%d)\n", led, x, y);
         LEDs().SetPixel(x, y, CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]));
      }
   }
   LEDs().Unlock();
   previousDataLength = length;

   if (sendFrame)
   {
      // LOGLN("[DMX] sending update");
      LEDs().Update();
      // Reset universeReceived to 0
      memset(universesReceived, 0, maxUniverses);
   }
}

void setup_artnet()
{
   LOGF("[DMX] Setting up Artnet controller\n");
   artnet.setArtDmxCallback(onDmxFrame);
   artnet.begin(HOSTNAME);
   LOGF("[DMX] Artnet setup done.\n");
}

void artnet_loop()
{
   artnet.read();
}
