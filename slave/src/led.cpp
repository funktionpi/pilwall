#include "led.h"

#include <Print.h>

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>
#include <FastLED.h>

// CRGB red(SATURATION, 0, 0);
// CRGB green(0, SATURATION, 0);
// CRGB blue(0, 0, SATURATION);
// CRGB white(SATURATION);
// CRGB black(0);

// CHSV hslRed(red);
// CHSV hslGreen(green);
// CHSV hslBlue(blue);
// CHSV hslWhite(white);
// CHSV hslBlack(black);

NeoMosaic<ColumnMajorAlternatingLayout> mosaic(
    MATRIX_TILE_WIDTH,
    MATRIX_TILE_HEIGHT,
    MATRIX_TILE_H,
    MATRIX_TILE_V);

// typedef NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32RmtSpeedWs2812x> NeoPixels;

// NeoPixels strip(MATRIX_SIZE, LED_PIN);

// NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip0(MATRIX_TILE_SIZE, 16);
// NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt1Ws2812xMethod> strip1(MATRIX_TILE_SIZE, LED_PIN);

// NeoPixels strips[]{
//     NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod>(MATRIX_TILE_SIZE, 16),
//     NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt1Ws2812xMethod>(MATRIX_TILE_SIZE, LED_PIN),
// };

// typedef CRGBArray<MATRIX_TILE_SIZE> NeoPixelStrip;
const int LedChannelWidth = MATRIX_SIZE;
// const int LedChannelWidth = MATRIX_TILE_SIZE;
typedef CRGBArray<LedChannelWidth> NeoPixelStrip;

NeoPixelStrip strips[]{
    NeoPixelStrip(),
    NeoPixelStrip(),
};

const int StripCount = sizeof(strips) / sizeof(NeoPixelStrip);

void led_show();

// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;

/** show() for ESP32
   Call this function instead of FastLED.show(). It signals core 0 to issue a show,
    then waits for a notification that it is done.
*/
void FastLEDshowESP32()
{
   if (userTaskHandle == 0)
   {
      // -- Store the handle of the current task, so that the show task can
      //    notify it when it's done
      userTaskHandle = xTaskGetCurrentTaskHandle();

      // -- Trigger the show task
      xTaskNotifyGive(FastLEDshowTaskHandle);

      // -- Wait to be notified that it's done
      const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200);
      ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
      userTaskHandle = 0;
   }
}

/** show Task
    This function runs on core 0 and just waits for requests to call FastLED.show()
*/
void FastLEDshowTask(void *pvParameters)
{
   // -- Run forever...
   for (;;)
   {
      // -- Wait for the trigger
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

      // -- Do the show (synchronously)
      FastLED.show();

      // -- Notify the calling task
      xTaskNotifyGive(userTaskHandle);
   }
}

void led_setup()
{
   pinMode(16, OUTPUT);
   pinMode(LED_PIN, OUTPUT);

   Serial.print("[LED] Matrix Size: ");
   Serial.print(MATRIX_WIDTH);
   Serial.print("x");
   Serial.println(MATRIX_HEIGHT);

   // for (int i = 0; i < StripCount; ++i)
   // {
   //    Serial.printf("[LED] Init strip %d\n", i);
   //    strips[i].Begin();
   // }
   // strip0.Begin();
   // strip1.Begin();

   FastLED.addLeds<WS2812B, 16, GRB>(strips[0], LedChannelWidth).setCorrection(TypicalSMD5050);
   // FastLED.addLeds<WS2812B, LED_PIN, GRB>(strips[1], LedChannelWidth).setCorrection(TypicalSMD5050);

   led_clear();
   led_set_brightness(BRIGHTNESS);

   // -- Create the FastLED show task
   xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, 0);

   Serial.println("[LED] setup done");
}

void led_show()
{
   // send the 'leds' array out to the actual LED strip
   FastLEDshowESP32();

   // strip0.Show();
   // strip1.Show();
   // for (int i = 0; i < StripCount; ++i)
   // {
   //    strips[i].Show();
   // }
}

void led_set_brightness(int brigth)
{
   FastLED.setBrightness(BRIGHTNESS);

   // strip0.SetBrightness(brigth);
   // strip1.SetBrightness(brigth);
   // for (int i = 0; i < StripCount; ++i)
   // {
   //    strips[i].SetBrightness(brigth);
   // }
}

void led_clear(CRGB col)
{
   // strip0.ClearTo(col);
   // strip1.ClearTo(col);
   for (int i = 0; i < StripCount; ++i)
   {
      strips[i].fill_solid(col);
   }
}

uint16_t led_get_width()
{
   return mosaic.getWidth();
}

uint16_t led_get_height()
{
   return mosaic.getHeight();
}

void led_set_pixel(uint16_t x, uint16_t y, CRGB color)
{
   auto idx = mosaic.Map(x, y);
   auto stripId = idx / LedChannelWidth;
   idx = idx % LedChannelWidth;
   strips[stripId][idx] = color;

   // delay(50);
   // Serial.printf("writing led %d on strip %d\n", idx, stripId);

   // if (stripId == 0)
   // {
   //    strip0.SetPixelColor(idx, color);
   // }
   // else
   // {
   //    strip1.SetPixelColor(idx, color);
   // }
}

void led_draw_line(int x1, int y1, int x2, int y2, CRGB col)
{
   // Serial.printf("[LED] draw line between (%d,%d) and (%d,%d)\n", x1, y1, x2, y2);

   for (size_t i = x1; i <= x2; i++)
   {
      for (int j = y1; j <= y2; j++)
      {
         led_set_pixel(i, j, col);
         // Serial.printf("[LED] setting color for (%d,%d), ledid = %d\n", i, j, id);
      }
   }
}

bool horizontal = true;
int it = 0;
int col = 0;
CRGB bgCols[] = {CRGB::Red, CRGB::Green, CRGB::Blue};

void led_cycle_pixels()
{
   led_clear(bgCols[col]);

   auto count = horizontal ? MATRIX_WIDTH : MATRIX_HEIGHT;

   Serial.printf("[LED] line %d / %d\n", it, count);

   if (horizontal)
   {
      led_draw_line(it, 0, it, MATRIX_HEIGHT, CRGB::White);
   }
   else
   {
      led_draw_line(0, it, MATRIX_WIDTH, it, CRGB::White);
   }

   it++;

   if (it >= count)
   {
      horizontal = !horizontal;
      it = 0;

      col = (col + 1) % 3;
      Serial.printf("[LED] now clearing to (R: %d, G: %d, B: %d)\n", bgCols[col].r, bgCols[col].g, bgCols[col].b);
   }
}

void led_flash_colors()
{
   FastLED.delay(25);
   led_clear(bgCols[col]);
   col = (col + 1) % 3;
}

void led_loop()
{
   // led_cycle_pixels();
   // led_flash_colors();
   led_show();
}