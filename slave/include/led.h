#ifndef _LED_H_
#define _LED_H_

#include <Arduino.h>
#include <FastLED.h>
#include <NeoPixelBus.h>

// #include <internal/RgbColor.h>

#define FIRST_LED_PIN 16
#define LED_CHANNEL_COUNT 2
#define DEFAULT_BRIGHTNESS 8 // 0 to 255

// Used by LEDMatrix
#define MATRIX_TILE_WIDTH 32 // width of EACH NEOPIXEL MATRIX (not total display)
#define MATRIX_TILE_HEIGHT 8 // height of each matrix
#define MATRIX_TILE_H 2      // number of matrices arranged horizontally
#define MATRIX_TILE_V 1      // number of matrices arranged vertically
#define MATRIX_TILE_SIZE (MATRIX_TILE_WIDTH * MATRIX_TILE_HEIGHT)
#define MATRIX_TILE_COUNT (MATRIX_TILE_H * MATRIX_TILE_V)

#define MATRIX_WIDTH (MATRIX_TILE_WIDTH * MATRIX_TILE_H)
#define MATRIX_HEIGHT (MATRIX_TILE_HEIGHT * MATRIX_TILE_V)
#define MATRIX_SIZE (MATRIX_WIDTH * MATRIX_HEIGHT)

const int LED_CHANNEL_WIDTH = MATRIX_SIZE / LED_CHANNEL_COUNT;

typedef NeoMosaic<ColumnMajorAlternatingLayout> Mosaic;

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
   virtual void Clear(uint32_t col = 0)  = 0;
   virtual void SetPixel(uint16_t x, uint16_t y, uint32_t color) = 0;
   virtual void Loop() = 0;
   virtual void SetBrightness(int brightness) = 0;

   virtual void DrawLine(int x1, int y1, int x2, int y2, uint32_t col);
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
   void Clear(uint32_t col = 0);
   void SetPixel(uint16_t x, uint16_t y, uint32_t color);
   void SetBrightness(int brigth);
   void Loop();

private:
   struct FastLedImpl *_impl;
};

class NeoPixelBusController : public LedController
{
public:
   NeoPixelBusController();
   virtual ~NeoPixelBusController() {}

   void Setup();
   void Clear(uint32_t col = 0);
   void SetPixel(uint16_t x, uint16_t y, uint32_t color);
   void SetBrightness(int brightness);
   void Loop();

private:
   struct NeoPixelBusImpl * _impl;
};

// Change this between FastLedController or NeoPixelBusController to change implementation
typedef FastLedController DefaultLedLibrary;

#endif