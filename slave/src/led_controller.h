#pragma once

#include "config.h"
#include <Arduino.h>
#include <Framebuffer_GFX.h>

class LedController : public Framebuffer_GFX
{
public:
   LedController();
   virtual ~LedController(){};

   virtual void setup() = 0;
   virtual void tick() = 0;

   virtual int XY(int16_t x, int16_t y);
   virtual void drawPixel(int16_t x, int16_t y, CRGB color) = 0;

   virtual void setBrightness(int brightness) = 0;
   virtual void copyRaw(int index, const uint8_t *src, int len) = 0;

   // Flip buffers
   virtual void update() = 0;

   virtual bool lock(bool block = true) = 0;
   virtual void unlock() = 0;

   virtual uint16_t width();
   virtual uint16_t height();

   void fillScreen(uint32_t color)
   {
      fillScreen(Framebuffer_GFX::Color24to16(color));
   }

   void show() {}

protected:
   Mosaic mosaic;
};

LedController &LEDs();
