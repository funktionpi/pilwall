#include <pixelset.h>

#include "fastled_ctrl.h"
#include "log.h"
#include "stopwatch.h"

#define FASTLED_SHOW_CORE 0
#define ENABLE_MUTEX 1
struct FastLedImpl
{
   volatile int maxFPS;
   TaskHandle_t FastLEDshowTaskHandle;

   SemaphoreHandle_t xMutex;

   NeoPixelStrip backbuffer;
   NeoPixelStrip frontbuffer;

   StopWatch drawWatch;
   StopWatch lockCore0Watch;
   StopWatch lockCore1Watch;

   CLEDController *controllers[LED_CHANNEL_COUNT];
};

void FastLEDshowTask(void *pvParameters)
{
   auto ctrl = (FastLedController *)pvParameters;
   ctrl->task();
}

FastLedController::FastLedController()
    : _impl(new FastLedImpl())
{
   _impl->lockCore0Watch.reset();
   _impl->lockCore1Watch.reset();
   _impl->drawWatch.reset();
   _impl->FastLEDshowTaskHandle = 0;
   newLedsPtr(_impl->backbuffer);
}

void FastLedController::setup()
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

   _impl->maxFPS = MAX_FPS;
   // FastLED.setMaxRefreshRate(60, false);
   FastLED.setCorrection(TypicalSMD5050);
   // FastLED.setDither(BINARY_DITHER);
   FastLED.clearData();

   // set max power to X amps * 5v
   // FastLED.setMaxPowerInVoltsAndMilliamps(5, POWER_AMPS * 1000);

   _impl->xMutex = xSemaphoreCreateMutex();

   auto pixsize = sizeof(CRGB);
   auto bufsize = pixsize * MATRIX_SIZE;
   auto arraysize = sizeof(CRGB[MATRIX_SIZE]);
   LOGF("[FLED] Memory size of pixel: %d bytes, Buffer size: %d bytes, Array size: %d\n", pixsize, bufsize, arraysize)

   // -- Create the FastLED show task
   xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, this, 1, &_impl->FastLEDshowTaskHandle, FASTLED_SHOW_CORE);

   LOGLN("[FLED] FastLED setup done")
}

void FastLedController::setBrightness(int brightness)
{
   lock(true);
   FastLED.setBrightness(brightness);
   unlock();
}

bool FastLedController::lock(bool block)
{
#if ENABLE_MUTEX
   if (!xPortGetCoreID()) _impl->lockCore0Watch.start();
   else _impl->lockCore1Watch.start();

   auto timeout = block ? portMAX_DELAY : pdMS_TO_TICKS(2);
   auto ret = xSemaphoreTake(_impl->xMutex, timeout) == pdTRUE;
   if (!ret)
   {
      LOGLN("[FLED] mutex lock timeout.")
   }

   if (!xPortGetCoreID()) _impl->lockCore0Watch.stop();
   else _impl->lockCore1Watch.stop();
   return ret;
#else
   return true;
#endif
}


void FastLedController::unlock()
{
#if ENABLE_MUTEX
   xSemaphoreGive(_impl->xMutex);
#endif
}

void FastLedController::copyRaw(int index, const uint8_t *src, int len)
{
   memcpy(_impl->backbuffer.leds + index, src, len * sizeof(CRGB));
}

void FastLedController::drawPixel(int16_t x, int16_t y, CRGB color)
{
   _impl->backbuffer[mosaic.Map(x, y)] = color;
}

void FastLedController::tick()
{
#ifdef SERIAL_DEBUG
   EVERY_N_SECONDS(5)
   {
      LOGF("[FLED] Current FPS: %d fps\n", FastLED.getFPS());
      if (_impl->drawWatch.runs())
      {
         LOGF("[FLED] Draw average: %d ms (%d runs)\n", _impl->drawWatch.averagems(), _impl->drawWatch.runs());
      }
      else
      {
         LOGLN("[FLED] ERROR: no draw was called (seems stuck)");
      }

      {
         auto runs = _impl->lockCore0Watch.runs();
         auto avg = _impl->lockCore0Watch.average();
         if (runs && avg)
         {
            LOGF("[FLED] Core0: mutex wait average: %d us (%d calls)\n", avg, runs);
            _impl->lockCore0Watch.reset();
         }
      }

            {
         auto runs = _impl->lockCore1Watch.runs();
         auto avg = _impl->lockCore1Watch.average();
         if (runs && avg)
         {
            LOGF("[FLED] Core1: mutex wait average: %d us (%d calls)\n", avg, runs);
            _impl->lockCore1Watch.reset();
         }
      }

      _impl->drawWatch.reset();
   }
#endif
}

void FastLedController::update()
{
   lock();
   memcpy(_impl->frontbuffer.leds, _impl->backbuffer.leds, MATRIX_SIZE * sizeof(CRGB));
   unlock();
}

void FastLedController::task()
{
   delay(250); // wait a little before starting to draw

   LOGF("[FLED] running FastLED task on core %d\n", xPortGetCoreID());

   FastLED.show();
   auto lastshow = micros();

   for(;;)
   {
      auto minMicros = 1000000 / _impl->maxFPS;

      while(minMicros && ((micros()-lastshow) < minMicros))
      {
         yield();
      }
      lastshow = micros();

      lock();
      _impl->drawWatch.start();
      FastLED.show();
      _impl->drawWatch.stop();
      unlock();
   }
}