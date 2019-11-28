#include "led.h"
#include "led_controller.h"
#include "log.h"

#define FASTLED_SHOW_CORE 0

typedef CRGBArray<MATRIX_SIZE> NeoPixelStrip;

struct FastLedImpl
{
   bool firstTick;
   volatile bool show;
   TaskHandle_t FastLEDshowTaskHandle;

   SemaphoreHandle_t xMutex;

   NeoPixelStrip backbuffer;
   NeoPixelStrip frontbuffer;

   CLEDController *controllers[LED_CHANNEL_COUNT];
};

void FastLEDshowTask(void *pvParameters)
{
   auto ctrl = (FastLedController *)pvParameters;
   ctrl->Task();
}

FastLedController::FastLedController()
    : _impl(new FastLedImpl())
{
   _impl->FastLEDshowTaskHandle = 0;
}

void FastLedController::Setup()
{
   LOGLN("[FLED] init FastLED library");

   for (int i = 0; i < LED_CHANNEL_COUNT; ++i)
   {
      pinMode(PINS[i], OUTPUT);
   }

   LOGLN("[FLED] init led frontbuffer #0");
   _impl->controllers[0] = &FastLED.addLeds<WS2812B, PIN_0, GRB>(_impl->frontbuffer, 0, LED_CHANNEL_WIDTH);

   if (LED_CHANNEL_COUNT > 1)
   {
      LOGLN("[FLED] init led frontbuffer #1");
      _impl->controllers[1] = &FastLED.addLeds<WS2812B, PIN_1, GRB>(_impl->frontbuffer, LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH);
   }
   if (LED_CHANNEL_COUNT > 2)
   {
      LOGLN("[FLED] init led frontbuffer #2");
      _impl->controllers[3] = &FastLED.addLeds<WS2812B, PIN_2, GRB>(_impl->frontbuffer, 2 * LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH);
   }
   if (LED_CHANNEL_COUNT > 3)
   {
      LOGLN("[FLED] init led frontbuffer #3");
      _impl->controllers[4] = &FastLED.addLeds<WS2812B, PIN_3, GRB>(_impl->frontbuffer, 3 * LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH);
   }

   FastLED.setCorrection(TypicalSMD5050);
   FastLED.setDither(DISABLE_DITHER);
   FastLED.clearData();

   // set max power to 30 amps * 5v
   FastLED.setMaxPowerInVoltsAndMilliamps(5, 30000);

   _impl->firstTick = true;
   _impl->xMutex = xSemaphoreCreateMutex();

   auto pixsize = sizeof(CRGB);
   auto bufsize = pixsize * MATRIX_SIZE;
   auto arraysize = sizeof(CRGB[MATRIX_SIZE]);
   LOGF("[FLED] Memory size of pixel: %d bytes, Buffer size: %d bytes, Array size: %d\n", pixsize, bufsize, arraysize)

   // -- Create the FastLED show task
   xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, this, 2, &_impl->FastLEDshowTaskHandle, FASTLED_SHOW_CORE);

   // FastLED.setMaxRefreshRate(60, false);

   LOGLN("[FLED] FastLED setup done")
}

void FastLedController::SetBrightness(int brightness)
{
   FastLED.setBrightness(brightness);
   LOGF("[FLED] brightness set to %d\n", brightness);
}

void FastLedController::Lock()
{
   // auto m = millis();
   // if (xPortInIsrContext())
   //    xSemaphoreTakeFromISR(_impl->xMutex, nullptr);
   // else
      xSemaphoreTake(_impl->xMutex, portMAX_DELAY);

   // auto diff = millis() - m;
   // if (diff > 0) {
   //    auto xHandle = xTaskGetCurrentTaskHandle();
   //    LOGF("[FLED] task %s blocked by mutex for  %dms\n", pcTaskGetTaskName(xHandle), int(diff));
   // }
}

void FastLedController::Unlock()
{
   // if (xPortInIsrContext())
   //    xSemaphoreGiveFromISR(_impl->xMutex, nullptr);
   // else
      xSemaphoreGive(_impl->xMutex);
}

void FastLedController::CopyRaw(int index, const char *src, int len)
{
   memcpy8(_impl->backbuffer.leds + index, src, len * sizeof(CRGB));
}

void FastLedController::Clear(CRGB color)
{
   _impl->backbuffer.fill_solid(CRGB(color));
}

void FastLedController::SetPixel(uint16_t x, uint16_t y, CRGB color)
{
   _impl->backbuffer[mosaic.Map(x, y)] = color;
}

void FastLedController::Tick()
{
#ifdef SERIAL_DEBUG
   EVERY_N_SECONDS(5)
   {
      LOGF("[FLED] Current FPS: %d fps\n", FastLED.getFPS());
   }
#endif
}

void FastLedController::Update()
{
   // _impl->show = true;
   xTaskNotifyGive(_impl->FastLEDshowTaskHandle);
}

void FastLedController::Task()
{
   delay(250); // wait a little before starting ops

   LOGF("[FLED] running FastLED task on core %d\n", xPortGetCoreID());

   for (;;)
   {
      if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1)) != 0)
      {
         Lock();
         memcpy8(_impl->frontbuffer.leds, _impl->backbuffer.leds, MATRIX_SIZE * sizeof(CRGB));
         Unlock();
      }

      FastLED.show();
   }
}