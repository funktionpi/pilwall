#include "led_controller.h"
#include "log.h"

#include <NeoPixelBrightnessBus.h>

#define NEOPIXEL_SHOW_CORE 0

typedef NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> NeoPixelChannel0;
typedef NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt1Ws2812xMethod> NeoPixelChannel1;
typedef NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt2Ws2812xMethod> NeoPixelChannel2;
typedef NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt3Ws2812xMethod> NeoPixelChannel3;

struct NeoPixelBusImpl
{
   NeoPixelChannel0 *strip0;
   NeoPixelChannel1 *strip1;
   NeoPixelChannel2 *strip2;
   NeoPixelChannel3 *strip3;
   SemaphoreHandle_t xMutex;
};

void NeoPixelshowTask(void *pvParameters);

static TaskHandle_t NeoPixelshowTaskHandle = 0;
static TaskHandle_t neopTaskHandle = 0;

NeoPixelBusController::NeoPixelBusController()
    : _impl(new NeoPixelBusImpl())
{
   memset(_impl, 0, sizeof(NeoPixelBusImpl)); // init to zero
}

void NeoPixelBusController::Setup()
{
   LOGLN("[LED] init NeoPixelBus library");

   _impl->strip0 = new NeoPixelChannel0(LED_CHANNEL_WIDTH, PIN_0);
   _impl->strip0->Begin();

   if (LED_CHANNEL_COUNT > 1)
   {
      _impl->strip1 = new NeoPixelChannel1(LED_CHANNEL_WIDTH, PIN_1);
      _impl->strip1->Begin();
   }

   if (LED_CHANNEL_COUNT > 2)
   {
      _impl->strip2 = new NeoPixelChannel2(LED_CHANNEL_WIDTH, PIN_2);
      _impl->strip2->Begin();
   }

   if (LED_CHANNEL_COUNT > 3)
   {
      _impl->strip3 = new NeoPixelChannel3(LED_CHANNEL_WIDTH, PIN_3);
      _impl->strip3->Begin();
   }

   _impl->xMutex = xSemaphoreCreateMutex();

   LOGLN("[LED] NeoPixel setup done")
}

void NeoPixelBusController::Update()
{
   _impl->strip0->Show();
   if (_impl->strip1) _impl->strip1->Show();
   if (_impl->strip2) _impl->strip2->Show();
   if (_impl->strip3) _impl->strip3->Show();
}

void NeoPixelBusController::Tick()
{
   // TODO
}

RgbColor adjustColor(int brightness, CRGB _color)
{
   CRGB tmp(_color);
   auto adjust = CLEDController::computeAdjustment(255, TypicalSMD5050, UncorrectedTemperature);

   auto out = RgbColor(
       tmp.r * adjust.r / 255,
       tmp.g * adjust.g / 255,
       tmp.b * adjust.b / 255);

   // LOGF("[LED] adjusting color (%d, %d, %d) by (%d, %d, %d) resulting in (%d, %d, %d)\n", tmp.r, tmp.g, tmp.b, adjust.r, adjust.g, adjust.b, out.R, out.G, out.B);

   return out;
}

void NeoPixelBusController::SetPixel(uint16_t x, uint16_t y, CRGB _color)
{
   auto color = adjustColor(_impl->strip0->GetBrightness(), _color);
   auto idx = mosaic.Map(x, y);
   auto stripId = idx / LED_CHANNEL_WIDTH;
   idx = idx % LED_CHANNEL_WIDTH;

   if (stripId == 0) _impl->strip0->SetPixelColor(idx, color);
   else if (stripId == 1) _impl->strip1->SetPixelColor(idx, color);
   else if (stripId == 2) _impl->strip2->SetPixelColor(idx, color);
   else if (stripId == 3) _impl->strip3->SetPixelColor(idx, color);
}

void NeoPixelBusController::CopyRaw(int index, const char *src, int len)
{

}

void NeoPixelBusController::Clear(CRGB _color)
{
   auto color = adjustColor(_impl->strip0->GetBrightness(), _color);
   _impl->strip0->ClearTo(color);
   if (_impl->strip1)
      _impl->strip1->ClearTo(color);
   if (_impl->strip2)
      _impl->strip2->ClearTo(color);
   if (_impl->strip3)
      _impl->strip3->ClearTo(color);
}

void NeoPixelBusController::Lock()
{
   xSemaphoreTake(_impl->xMutex, portMAX_DELAY);
}

void NeoPixelBusController::Unlock()
{
   xSemaphoreGive(_impl->xMutex);
}

void NeoPixelBusController::SetBrightness(int brightness)
{
   _impl->strip0->SetBrightness(brightness);
   if (_impl->strip1) _impl->strip1->SetBrightness(brightness);
   if (_impl->strip2) _impl->strip2->SetBrightness(brightness);
   if (_impl->strip3) _impl->strip3->SetBrightness(brightness);
}

/** show Task
    This function runs on core 0 and just waits for requests to call FastLED.show()
*/
void NeoPixelshowTask(void *pvParameters)
{
   LOGF("[LED] Neopixel loop on core %d\n", xPortGetCoreID());
   auto ctrl = (NeoPixelBusController *)pvParameters;
   ctrl->LoopTask();
}