#pragma once

#include "led_controller.h"
#include <FastLED.h>

typedef CRGBArray<MATRIX_SIZE> NeoPixelStrip;

class FastLedController : public LedController
{
public:
   FastLedController();
   virtual ~FastLedController() {}

   void setup();
   void tick();

   void setBrightness(int brightness);
   void drawPixel(int16_t x, int16_t y, CRGB color);
   void copyRaw(int index, const uint8_t *src, int len);

   void update();
   bool lock(bool block = true);
   void unlock();

   void task();

private:
   NeoPixelStrip* buffer();

   struct FastLedImpl *_impl;
};