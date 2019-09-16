#include "led.h"

#include <Print.h>

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>
#include <FastLED.h>


void led_show();

DefaultLedLibrary controller;

void led_setup()
{
   Serial.printf("[LED] Matrix Size: %dx%d\n", MATRIX_WIDTH, MATRIX_HEIGHT);
   Serial.printf("[LED] Using %d channels with %d pixels\n", LED_CHANNEL_COUNT, LED_CHANNEL_WIDTH);

   controller.Setup();
   controller.SetBrightness(DEFAULT_BRIGHTNESS);

   Serial.println("[LED] setup done");
}

void led_show()
{
   controller.Loop();
}

LedController& LEDs()
{
   return controller;
}

bool horizontal = true;
int it = 0;
int col = 0;
CRGB bgCols[] = {CRGB::Red, CRGB::Black, CRGB::Green, CRGB::Black, CRGB::Blue, CRGB::Black};

void led_cycle_pixels()
{
   controller.Clear(colorToInt(bgCols[col]));

   auto count = horizontal ? MATRIX_WIDTH : MATRIX_HEIGHT;

   // Serial.printf("[LED] line %d / %d\n", it, count);

   if (horizontal)
   {
      controller.DrawLine(it, 0, it, MATRIX_HEIGHT, colorToInt(CRGB::White));
   }
   else
   {
      controller.DrawLine(0, it, MATRIX_WIDTH, it, colorToInt(CRGB::White));
   }

   it++;

   if (it >= count)
   {
      horizontal = !horizontal;
      it = 0;

      col = (col + 1) % (sizeof(bgCols) / sizeof(CRGB));
      Serial.printf("[LED] now clearing to (R: %d, G: %d, B: %d)\n", bgCols[col].r, bgCols[col].g, bgCols[col].b);
   }
}

void led_flash_colors()
{
   FastLED.delay(50);
   auto color = bgCols[col];
   controller.Clear(colorToInt(color));
   col = (col + 1) % (sizeof(bgCols) / sizeof(CRGB));
}

void led_loop()
{
   Serial.println("led loop");
   led_cycle_pixels();
   // led_flash_colors();
   controller.Loop();
}