#include "led.h"
#include "log.h"

// #define FASTLED_RMT_BUILTIN_DRIVER true
#include <FastLED.h>

#define FASTLED_SHOW_CORE 0

typedef CRGBArray<MATRIX_SIZE> NeoPixelStrip;

// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;

struct FastLedImpl
{
   NeoPixelStrip strip;
};


SemaphoreHandle_t xSemaphore;

struct MutexLock {
   MutexLock() {
      xSemaphoreTake(xSemaphore, portMAX_DELAY);
   }

   ~MutexLock() {
      xSemaphoreGive(xSemaphore);
   }
};

void FastLEDshowTask(void *pvParameters);

FastLedController::FastLedController()
    : _impl(new FastLedImpl())
{
}

void FastLedController::Setup()
{
   LOGLN("[LED] init FastLED library");

   for (int pin = FIRST_LED_PIN; pin < LED_CHANNEL_COUNT; ++pin)
   {
      pinMode(pin, OUTPUT);
   }

   LOGLN("[LED] init led strip #0");
   FastLED.addLeds<WS2812B, FIRST_LED_PIN, GRB>(_impl->strip, 0, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);

   if (LED_CHANNEL_COUNT > 1)
   {
      LOGLN("[LED] init led strip #1");
      FastLED.addLeds<WS2812B, FIRST_LED_PIN + 1, GRB>(_impl->strip, LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }
   if (LED_CHANNEL_COUNT > 2)
   {
      LOGLN("[LED] init led strip #2");
      FastLED.addLeds<WS2812B, FIRST_LED_PIN + 2, GRB>(_impl->strip, 2 * LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }
   if (LED_CHANNEL_COUNT > 3)
   {
      LOGLN("[LED] init led strip #3");
      FastLED.addLeds<WS2812B, FIRST_LED_PIN + 3, GRB>(_impl->strip, 3 * LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }

   xSemaphore = xSemaphoreCreateMutex();
   xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 10000, NULL,2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);

   FastLED.clearData();
}

void FastLedController::SetBrightness(int brightness)
{
   MutexLock(xSemaphore);
   FastLED.setBrightness(brightness);

}

void FastLedController::Clear(CRGB color)
{
   MutexLock(xSemaphore);
   _impl->strip.fill_solid(CRGB(color));
}

void FastLedController::SetPixel(uint16_t x, uint16_t y, CRGB color)
{
   MutexLock(xSemaphore);
   _impl->strip[mosaic.Map(x, y)] = color;
}

/** show() for ESP32
   Call this function instead of FastLED.show(). It signals core 0 to issue a show,
    then waits for a notification that it is done.
*/
void FastLedController::Loop()
{
   // if (userTaskHandle == 0)
   // {
      // -- Store the handle of the current task, so that the show task can
      //    notify it when it's done
      // userTaskHandle = xTaskGetCurrentTaskHandle();

      // -- Trigger the show task
      // xTaskNotifyGive(FastLEDshowTaskHandle);

      // -- Wait to be notified that it's done
      // const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200);
      // ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
      // userTaskHandle = 0;
   // }
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
      // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

      // -- Do the show (synchronously)
      // xSemaphoreTake(xSemaphore, portMAX_DELAY);
      FastLED.show();
      // xSemaphoreGive(xSemaphore);

      // -- Notify the calling task
      // xTaskNotifyGive(userTaskHandle);
   }
}