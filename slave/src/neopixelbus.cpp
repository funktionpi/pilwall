#include "led.h"
#include <NeoPixelBrightnessBus.h>

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
};

NeoPixelBusController::NeoPixelBusController()
    : _impl(new NeoPixelBusImpl())
{
   memset(_impl, 0, sizeof(NeoPixelBusImpl)); // init to zero
}

void NeoPixelBusController::Setup()
{
   Serial.println("[LED] init NeoPixelBus library");

   _impl->strip0 = new NeoPixelChannel0(LED_CHANNEL_WIDTH, FIRST_LED_PIN);
   _impl->strip0->Begin();

   if (LED_CHANNEL_COUNT > 1)
   {
      _impl->strip1 = new NeoPixelChannel1(LED_CHANNEL_WIDTH, FIRST_LED_PIN+1);
      _impl->strip1->Begin();
   }

   if (LED_CHANNEL_COUNT > 2)
   {
      _impl->strip2 = new NeoPixelChannel2(LED_CHANNEL_WIDTH, FIRST_LED_PIN+2);
      _impl->strip2->Begin();
   }

   if (LED_CHANNEL_COUNT > 3)
   {
      _impl->strip3 = new NeoPixelChannel3(LED_CHANNEL_WIDTH, FIRST_LED_PIN+3);
      _impl->strip3->Begin();
   }
}

void NeoPixelBusController::Loop()
{
   _impl->strip0->Show();
   if (_impl->strip1) _impl->strip1->Show();
   if (_impl->strip2) _impl->strip2->Show();
   if (_impl->strip3) _impl->strip3->Show();
}

RgbColor adjustColor(int brightness, uint32_t _color)
{
   CRGB tmp (_color);
   auto adjust = CLEDController::computeAdjustment(255, TypicalSMD5050, UncorrectedTemperature);

   auto out = RgbColor(
       tmp.r * adjust.r / 255,
       tmp.g * adjust.g / 255,
       tmp.b * adjust.b / 255);

   // Serial.printf("[LED] adjusting color (%d, %d, %d) by (%d, %d, %d) resulting in (%d, %d, %d)\n", tmp.r, tmp.g, tmp.b, adjust.r, adjust.g, adjust.b, out.R, out.G, out.B);

   return out;
}

void NeoPixelBusController::SetPixel(uint16_t x, uint16_t y, uint32_t _color)
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

void NeoPixelBusController::Clear(uint32_t _color)
{
   // CRGB tmp(_color);
   // Serial.printf("[LED] now clearing to (R: %d, G: %d, B: %d)\n", tmp.r, tmp.g, tmp.b);

   auto color = adjustColor(_impl->strip0->GetBrightness(), _color);

   _impl->strip0->ClearTo(color);
   if (_impl->strip1) _impl->strip1->ClearTo(color);
   if (_impl->strip2) _impl->strip2->ClearTo(color);
   if (_impl->strip3) _impl->strip3->ClearTo(color);
}

void NeoPixelBusController::SetBrightness(int brightness)
{
   _impl->strip0->SetBrightness(brightness);
   if (_impl->strip1) _impl->strip1->SetBrightness(brightness);
   if (_impl->strip2) _impl->strip2->SetBrightness(brightness);
   if (_impl->strip3) _impl->strip3->SetBrightness(brightness);
}