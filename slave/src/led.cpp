#include "led.h"
#include "led_controller.h"
#include "log.h"

// Change this between FastLedController or NeoPixelBusController to change implementation
#if USE_FASTLED
#include "fastled_ctrl.h"
FastLedController controller;
#endif

#if USE_NEOPIXELBUS
#include "neopixelbus_ctrl.h"
NeoPixelBusController controller;
#endif

void setup_led()
{
   LOGF("[LED] Matrix Size: %dx%d\n", MATRIX_WIDTH, MATRIX_HEIGHT);
   LOGF("[LED] Using %d channels with %d pixels for a total of %d pixels\n", LED_CHANNEL_COUNT, LED_CHANNEL_WIDTH, MATRIX_SIZE);

   controller.setup();
   // controller.SetBrightness(DEFAULT_BRIGHTNESS);
   controller.clear();
   controller.update();

   LOGLN("[LED] setup done");
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
   EVERY_N_MILLISECONDS(50)
   {
      controller.fillScreen(bgCols[col]);

      auto count = horizontal ? MATRIX_WIDTH : MATRIX_HEIGHT;

      // LOGF("[LED] line %d / %d\n", it, count);

      if (horizontal)
      {
         controller.drawLine(it, 0, it, MATRIX_HEIGHT, LedController::Color24to16(CRGB::White));
      }
      else
      {
         controller.drawLine(0, it, MATRIX_WIDTH, it, LedController::Color24to16(CRGB::White));
      }

      it++;

      if (it >= count)
      {
         horizontal = !horizontal;
         it = 0;

         col = (col + 1) % (sizeof(bgCols) / sizeof(CRGB));
         // LOGF("[LED] now clearing to (R: %d, G: %d, B: %d)\n", bgCols[col].r, bgCols[col].g, bgCols[col].b);
      }
      controller.update();
   }
}

void led_flash_colors()
{
   EVERY_N_MILLISECONDS(250)
   {
      auto color = bgCols[col];

      controller.Clear(colorToInt(color));

      // controller.DrawLine(0, 0, MATRIX_WIDTH, 0, bgCols[col]);
      // controller.DrawLine(0, 1, MATRIX_WIDTH, 1, bgCols[col]);

      col = (col + 1) % (sizeof(bgCols) / sizeof(CRGB));

      controller.Update();
   }
}

void tick_led()
{
   // led_cycle_pixels();
   // led_flash_colors();
   controller.tick();
}