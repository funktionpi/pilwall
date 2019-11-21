#include "led.h"
#include "log.h"
#include "mutex.h"

#undef FASTLED_HAS_PRAGMA_MESSAGE
#include <FastLED.h>

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define FASTLED_SHOW_CORE 0

typedef CRGBArray<MATRIX_SIZE> NeoPixelStrip;

struct FastLedImpl
{
   bool firstTick;
   bool show;
   TaskHandle_t FastLEDshowTaskHandle;
   TaskHandle_t userTaskHandle;

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
   _impl->userTaskHandle = 0;
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
   _impl->controllers[0] = &FastLED.addLeds<WS2812B, PIN_0, GRB>(_impl->frontbuffer, 0, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);

   if (LED_CHANNEL_COUNT > 1)
   {
      LOGLN("[FLED] init led frontbuffer #1");
      _impl->controllers[1] = &FastLED.addLeds<WS2812B, PIN_1, GRB>(_impl->frontbuffer, LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }
   if (LED_CHANNEL_COUNT > 2)
   {
      LOGLN("[FLED] init led frontbuffer #2");
      _impl->controllers[3] = &FastLED.addLeds<WS2812B, PIN_2, GRB>(_impl->frontbuffer, 2 * LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }
   if (LED_CHANNEL_COUNT > 3)
   {
      LOGLN("[FLED] init led frontbuffer #3");
      _impl->controllers[4] = &FastLED.addLeds<WS2812B, PIN_3, GRB>(_impl->frontbuffer, 3 * LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH).setCorrection(TypicalSMD5050);
   }

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

   FastLED.setMaxRefreshRate(60, false);

   LOGLN("[FLED] FastLED setup done")
}

void FastLedController::SetBrightness(int brightness)
{
   FastLED.setBrightness(brightness);
   LOGF("[FLED] brightness set to %d\n", brightness);
}

void FastLedController::Lock()
{
   xSemaphoreTake(_impl->xMutex, portMAX_DELAY);
}

void FastLedController::Unlock()
{
   xSemaphoreGive(_impl->xMutex);
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
   // -- Trigger the show task
   xTaskNotifyGive(_impl->FastLEDshowTaskHandle);
}

void FastLedController::Task()
{
   FastLED.delay(250); // wait at least half a second before processing request

   LOGF("[FLED] running FastLED task on core %d\n", xPortGetCoreID());

   for (;;)
   {
      // sleep until notified
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

      // copy backbuffer to front
      Lock();
      memcpy8(_impl->frontbuffer.leds, _impl->backbuffer.leds, MATRIX_SIZE * sizeof(struct CRGB));
      Unlock();

      FastLED.show();
   }
}