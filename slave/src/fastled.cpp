#include "led.h"
#include "log.h"
#include "mutex.h"

// #define FASTLED_RMT_BUILTIN_DRIVER true
#include <FastLED.h>

#define FASTLED_SHOW_CORE 0

typedef CRGBArray<MATRIX_SIZE> NeoPixelStrip;

// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t fastledTaskHandle = 0;

struct FastLedImpl
{
   bool needUpdate;
   NeoPixelStrip strip;
   SemaphoreHandle_t xMutex;
};


void FastLEDshowTask(void *pvParameters);

FastLedController::FastLedController()
    : _impl(new FastLedImpl())
{
}

void FastLedController::Setup()
{
   LOGLN("[FLED] init FastLED library");

   for (int i = 0; i < LED_CHANNEL_COUNT; ++i)
   {
      pinMode(PINS[i], OUTPUT);
   }

   LOGLN("[FLED] init led strip #0");
   FastLED.addLeds<WS2812B, PIN_0, GRB>(_impl->strip, 0, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);

   if (LED_CHANNEL_COUNT > 1)
   {
      LOGLN("[FLED] init led strip #1");
      FastLED.addLeds<WS2812B, PIN_1, GRB>(_impl->strip, LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }
   if (LED_CHANNEL_COUNT > 2)
   {
      LOGLN("[FLED] init led strip #2");
      FastLED.addLeds<WS2812B, PIN_2, GRB>(_impl->strip, 2 * LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }
   if (LED_CHANNEL_COUNT > 3)
   {
      LOGLN("[FLED] init led strip #3");
      FastLED.addLeds<WS2812B, PIN_3, GRB>(_impl->strip, 3 * LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }

   _impl->xMutex = xSemaphoreCreateRecursiveMutex();
   // xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 10000, this, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);

   FastLED.clearData();

   // set max power to 30 amps * 5v
   FastLED.setMaxPowerInVoltsAndMilliamps(5, 30000);

   LOGLN("[FLED] FastLED setup done")
}

void FastLedController::SetBrightness(int brightness)
{
   AUTO_MUTEX
   FastLED.setBrightness(brightness);
   LOGF("[FLED] brightness set to %d\n", brightness);
}

void FastLedController::Clear(CRGB color)
{
   AUTO_MUTEX
   _impl->strip.fill_solid(CRGB(color));
}

void FastLedController::SetPixel(uint16_t x, uint16_t y, CRGB color)
{
   AUTO_MUTEX
   _impl->strip[mosaic.Map(x, y)] = color;
}

/** show() for ESP32
   Call this function instead of FastLED.show(). It signals core 0 to issue a show,
    then waits for a notification that it is done.
*/
void FastLedController::Tick()
{
   #ifdef SERIAL_DEBUG
   EVERY_N_SECONDS(5) {
      LOGF("[FLED] Current FPS: %d fps\n", FastLED.getFPS());
   }
   #endif

   /*
   if (fastledTaskHandle == 0)
   {
      // -- Store the handle of the current task, so that the show task can
      //    notify it when it's done
      fastledTaskHandle = xTaskGetCurrentTaskHandle();

      // -- Trigger the show task
      xTaskNotifyGive(FastLEDshowTaskHandle);

      // -- Wait to be notified that it's done
      const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200);
      ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
      fastledTaskHandle = 0;
   }
   */
}

void FastLedController::Update()
{
   FastLED.show();
}

void FastLedController::LoopTask()
{
   // -- Run forever...
   for (;;)
   {
      #ifdef SERIAL_DEBUG
      EVERY_N_SECONDS(5) {
         LOGF("[FLED] Current FPS: %d fps\n", FastLED.getFPS());
      }
      #endif

      if (_impl->needUpdate)
      {
         _impl->needUpdate = false;
         FastLED.show();
      }

      // -- Wait for the trigger
      // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

      // -- Do the show (synchronously) at MAX 60 fps
      // EVERY_N_MILLISECONDS(17) {
      //    AUTO_MUTEX
      //    // xSemaphoreTakeRecursive(xMutex, portMAX_DELAY);
      //    // LOGLN("sending update pixels")
      //    FastLED.show();
      //    // xSemaphoreGiveRecursive(xMutex);
      // }

      // -- Notify the calling task
      // xTaskNotifyGive(fastledTaskHandle);
   }
}

/** show Task
    This function runs on another core and calls the Show function
    there might be threading issues
*/
void FastLEDshowTask(void *pvParameters)
{
   auto controller = (FastLedController *)pvParameters;
   controller->LoopTask();
}