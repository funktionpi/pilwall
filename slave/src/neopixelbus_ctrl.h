#pragma once

#include "config.h"

#if USE_NEOPIXELBUS

#include "led_controller.h"

class NeoPixelBusController : public LedController
{
public:
   NeoPixelBusController();
   virtual ~NeoPixelBusController() {}

   void setup();
   void clear(uint16_t rgb);
   void drawPixel(uint16_t x, uint16_t y, uint16_t color);
   void setBrightness(int brightness);
   void copyRaw(int index, const uint8_t *src, int len);
   void tick();
   void update();

   bool lock(bool wait);
   void unlock();

   void LoopTask();

private:
   struct NeoPixelBusImpl * _impl;
};

#endif