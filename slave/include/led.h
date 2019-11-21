#ifndef _LED_H_
#define _LED_H_

#include <Arduino.h>

#undef FASTLED_HAS_PRAGMA_MESSAGE
#include <FastLED.h>

#include <NeoPixelBus.h>

#define PIN_0 4
#define PIN_1 19
#define PIN_2 33
#define PIN_3 32

const int PINS[] = {PIN_0, PIN_1, PIN_2, PIN_3};

#define LED_CHANNEL_COUNT 4
#define DEFAULT_BRIGHTNESS 64 // 0 to 255

// Used by LEDMatrix
#define MATRIX_TILE_WIDTH 32 // width of EACH NEOPIXEL MATRIX (not total display)
#define MATRIX_TILE_HEIGHT 8 // height of each matrix
#define MATRIX_TILE_H 2      // number of matrices arranged horizontally
#define MATRIX_TILE_V 2      // number of matrices arranged vertically
#define MATRIX_TILE_SIZE (MATRIX_TILE_WIDTH * MATRIX_TILE_HEIGHT)
#define MATRIX_TILE_COUNT (MATRIX_TILE_H * MATRIX_TILE_V)

#define MATRIX_WIDTH (MATRIX_TILE_WIDTH * MATRIX_TILE_H)
#define MATRIX_HEIGHT (MATRIX_TILE_HEIGHT * MATRIX_TILE_V)
#define MATRIX_SIZE (MATRIX_WIDTH * MATRIX_HEIGHT)

const int LED_CHANNEL_WIDTH = MATRIX_SIZE / LED_CHANNEL_COUNT;

#define AUTO_MUTEX /*MutexLockRecursive mutex(_impl->xMutex);*/

// typedef NeoMosaic<ColumnMajorAlternatingTilePreference> Mosaic;
typedef NeoTiles<ColumnMajorAlternatingLayout , RowMajorLayout> Mosaic;

void led_setup();
void led_loop();

inline uint32_t colorToInt(CRGB col)
{
   return (col.r << 16) | (col.g << 8) | col.b;
}

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
   void SetPixel(uint16_t x, uint16_t y, CRGB color);
   void SetBrightness(int brigth);
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
   void Tick();
   void Update();

   void Lock();
   void Unlock();

   void LoopTask();

private:
   struct NeoPixelBusImpl * _impl;
};

// Change this between FastLedController or NeoPixelBusController to change implementation
typedef FastLedController DefaultLedLibrary;

#endif