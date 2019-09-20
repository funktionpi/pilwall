#include "led.h"
#include "log.h"

#include <Print.h>

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>


void led_show();

DefaultLedLibrary controller;

void led_setup()
{
   LOGF("[LED] Matrix Size: %dx%d\n", MATRIX_WIDTH, MATRIX_HEIGHT);
   LOGF("[LED] Using %d channels with %d pixels\n", LED_CHANNEL_COUNT, LED_CHANNEL_WIDTH);

   controller.Setup();
   controller.SetBrightness(DEFAULT_BRIGHTNESS);

   LOGLN("[LED] setup done");
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

   delay(20);

   auto count = horizontal ? MATRIX_WIDTH : MATRIX_HEIGHT;

   // LOGF("[LED] line %d / %d\n", it, count);

   if (horizontal)
   {
      controller.DrawLine(it, 0, it, MATRIX_HEIGHT, CRGB::White);
   }
   else
   {
      controller.DrawLine(0, it, MATRIX_WIDTH, it, CRGB::White);
   }

   it++;

   if (it >= count)
   {
      horizontal = !horizontal;
      it = 0;

      col = (col + 1) % (sizeof(bgCols) / sizeof(CRGB));
      LOGF("[LED] now clearing to (R: %d, G: %d, B: %d)\n", bgCols[col].r, bgCols[col].g, bgCols[col].b);
   }
}

void led_flash_colors()
{
   delay(50);
   auto color = bgCols[col];
   controller.Clear(colorToInt(color));

   // controller.DrawLine(0, 0, MATRIX_WIDTH, 0, bgCols[col]);
   // controller.DrawLine(0, 1, MATRIX_WIDTH, 1, bgCols[col]);

   col = (col + 1) % (sizeof(bgCols) / sizeof(CRGB));
}

void led_loop()
{
   // led_cycle_pixels();
   // led_flash_colors();
   controller.Loop();
}