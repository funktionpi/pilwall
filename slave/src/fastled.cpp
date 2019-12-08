#include "led.h"
#include "led_controller.h"
#include "log.h"
#include "stopwatch.h"

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

   StopWatch lockWatch;
   StopWatch drawWatch;

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
   _impl->lockWatch.reset();
   _impl->drawWatch.reset();
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
      // _impl->controllers[3]->setTemperature(MercuryVapor);
   }
   if (LED_CHANNEL_COUNT > 3)
   {
      LOGLN("[FLED] init led frontbuffer #3");
      _impl->controllers[4] = &FastLED.addLeds<WS2812B, PIN_3, GRB>(_impl->frontbuffer, 3 * LED_CHANNEL_WIDTH, LED_CHANNEL_WIDTH);
   }

   FastLED.setCorrection(TypicalSMD5050);
   FastLED.setDither(BINARY_DITHER);
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
   xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, this, 1, &_impl->FastLEDshowTaskHandle, FASTLED_SHOW_CORE);

   // FastLED.setMaxRefreshRate(60, false);

   FastLED.show();

   LOGLN("[FLED] FastLED setup done")
}

void FastLedController::SetBrightness(int brightness)
{
   FastLED.setBrightness(brightness);
   // LOGF("[FLED] brightness set to %d\n", brightness);
}

void FastLedController::Lock()
{
   // _impl->lockWatch.start();
   // xSemaphoreTake(_impl->xMutex, portMAX_DELAY);
   // _impl->lockWatch.stop();
}

void FastLedController::Unlock()
{
   // xSemaphoreGive(_impl->xMutex);
}

void FastLedController::CopyRaw(int index, const uint8_t *src, int len)
{
   memcpy(_impl->backbuffer.leds + index, src, len * sizeof(CRGB));
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
      // LOGF("[FLED] Lock average: %d us (%d runs)\n", _impl->lockWatch.average(), _impl->lockWatch.runs());
      // _impl->lockWatch.reset();
      LOGF("[FLED] Draw average: %d ms (%d runs)\n", _impl->drawWatch.averagems(), _impl->drawWatch.runs());
      _impl->drawWatch.reset();
   }
#endif
}

void FastLedController::Update()
{
   xTaskNotifyGive(_impl->FastLEDshowTaskHandle);
//   memcpy(_impl->frontbuffer.leds, _impl->backbuffer.leds, MATRIX_SIZE * sizeof(CRGB));
}

void FastLedController::Task()
{
   delay(250); // wait a little before starting to draw

   LOGF("[FLED] running FastLED task on core %d\n", xPortGetCoreID());

   for (;;)
   {
      if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1)) != pdTRUE)
      {
         // Lock();
         memcpy(_impl->frontbuffer.leds, _impl->backbuffer.leds, MATRIX_SIZE * sizeof(CRGB));
         // Unlock();
      }

      _impl->drawWatch.start();
      FastLED.show();
      _impl->drawWatch.stop();
      taskYIELD();
   }
}