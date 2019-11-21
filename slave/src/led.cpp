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
   LOGF("[LED] Using %d channels with %d pixels for a total of %d pixels\n", LED_CHANNEL_COUNT, LED_CHANNEL_WIDTH, MATRIX_SIZE);

   controller.Setup();
   controller.SetBrightness(DEFAULT_BRIGHTNESS);
   controller.Clear(0);
   controller.Update();

   LOGLN("[LED] setup done");
}

void led_show()
{
   controller.Tick();
}

LedController &LEDs()
{
   return controller;
}

bool horizontal = true;
int it = 0;
int col = 0;
CRGB bgCols[] = {CRGB::Red, CRGB::Black, CRGB::Green, CRGB::Black, CRGB::Blue, CRGB::Black};

void led_cycle_pixels()
{
   EVERY_N_MILLISECONDS(5)
   {
      controller.Lock();
      controller.Clear(colorToInt(bgCols[col]));

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
         // LOGF("[LED] now clearing to (R: %d, G: %d, B: %d)\n", bgCols[col].r, bgCols[col].g, bgCols[col].b);
      }

      controller.Unlock();
      controller.Update();
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

   controller.Update();
}

void led_loop()
{
   // led_cycle_pixels();
   // led_flash_colors();
   controller.Tick();
}