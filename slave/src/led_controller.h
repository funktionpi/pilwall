#ifndef __LED_CONTROLLER_H__
#define __LED_CONTROLLER_H__

#include <Arduino.h>

#undef FASTLED_HAS_PRAGMA_MESSAGE
// #define FASTLED_RMT_MAX_CHANNELS=4
// #define FASTLED_RMT_BUILTIN_DRIVER=1
// #define FASTLED_ALLOW_INTERRUPTS 0
// #define FASTLED_INTERRUPT_RETRY_COUNT
#define FASTLED_ESP32_I2S 1
#include <FastLED.h>
#include <NeoPixelBus.h>

#include "config.h"

// typedef NeoMosaic<ColumnMajorAlternatingTilePreference> Mosaic;
typedef NeoTiles<ColumnMajorAlternatingLayout , RowMajorLayout> Mosaic;

class FastLedController;
class NeoPixelBusController;

// Change this between FastLedController or NeoPixelBusController to change implementation
typedef FastLedController DefaultLedLibrary;

class LedController
{
public:
   LedController();
   virtual ~LedController() {};

   virtual void Setup() = 0;
   virtual void Clear(CRGB rgb)  = 0;
   virtual void SetPixel(uint16_t x, uint16_t y, CRGB color) = 0;
   virtual void Tick() = 0;
   virtual void SetBrightness(int brightness) = 0;
   virtual void Update() = 0;
   virtual void CopyRaw(int index, const char *src, int len) = 0;

   virtual void Lock() = 0;
   virtual void Unlock() = 0;

   virtual void DrawLine(int x1, int y1, int x2, int y2, CRGB col);
   virtual uint16_t Width();
   virtual uint16_t Height();

protected:
   Mosaic mosaic;
};

LedController &LEDs();

class FastLedController : public LedController
{
public:
   FastLedController();
   virtual ~FastLedController() {}

   void Setup();
   void Clear(CRGB rgb);
   void SetPixels(uint16_t index, CRGB* colors, int count);
   void SetPixel(uint16_t x, uint16_t y, CRGB color);
   void SetBrightness(int brigth);
   void CopyRaw(int index, const char *src, int len);
   void Tick();
   void Update();

   void Lock();
   void Unlock();

   void Task();
private:
   struct FastLedImpl *_impl;
};

class NeoPixelBusController : public LedController
{
public:
   NeoPixelBusController();
   virtual ~NeoPixelBusController() {}

   void Setup();
   void Clear(CRGB rgb);
   void SetPixel(uint16_t x, uint16_t y, CRGB color);
   void SetBrightness(int brightness);
   void CopyRaw(int index, const char *src, int len);
   void Tick();
   void Update();

   void Lock();
   void Unlock();

   void LoopTask();

private:
   struct NeoPixelBusImpl * _impl;
};

inline uint32_t colorToInt(CRGB col)
{
   return (col.r << 16) | (col.g << 8) | col.b;
}

#endif