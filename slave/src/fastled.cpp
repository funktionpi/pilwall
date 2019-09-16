#include "led.h"
#include <FastLED.h>

#define FASTLED_SHOW_CORE 1

typedef CRGBArray<MATRIX_SIZE> NeoPixelStrip;

// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;

struct FastLedImpl
{
   NeoPixelStrip strip;
};

void FastLEDshowTask(void *pvParameters);

FastLedController::FastLedController()
    : _impl(new FastLedImpl())
{
}

void FastLedController::Setup()
{
   Serial.println("[LED] init FastLED library");

   for (int pin = FIRST_LED_PIN; pin < LED_CHANNEL_COUNT; ++pin)
   {
      pinMode(pin, OUTPUT);
   }

   Serial.println("[LED] init led strip #0");
   FastLED.addLeds<WS2812B, FIRST_LED_PIN, GRB>(_impl->strip, 0, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);

   if (LED_CHANNEL_COUNT > 1)
   {
      Serial.println("[LED] init led strip #1");
      FastLED.addLeds<WS2812B, FIRST_LED_PIN + 1, GRB>(_impl->strip, LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }
   if (LED_CHANNEL_COUNT > 2)
   {
      Serial.println("[LED] init led strip #2");
      FastLED.addLeds<WS2812B, FIRST_LED_PIN + 2, GRB>(_impl->strip, 2 * LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }
   if (LED_CHANNEL_COUNT > 3)
   {
      Serial.println("[LED] init led strip #3");
      FastLED.addLeds<WS2812B, FIRST_LED_PIN + 3, GRB>(_impl->strip, 3 * LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }

   xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 10000, NULL,2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);

   FastLED.clearData();
}

void FastLedController::SetBrightness(int brightness)
{
   FastLED.setBrightness(brightness);
}

void FastLedController::Clear(uint32_t color)
{
   _impl->strip.fill_solid(CRGB(color));
}

void FastLedController::SetPixel(uint16_t x, uint16_t y, uint32_t color)
{
   _impl->strip[mosaic.Map(x, y)] = CRGB(color);
}

/** show() for ESP32
   Call this function instead of FastLED.show(). It signals core 0 to issue a show,
    then waits for a notification that it is done.
*/
void FastLedController::Loop()
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