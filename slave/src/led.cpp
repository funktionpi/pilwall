#include "led.h"

#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>
#include <LEDMatrix.h>
#include <Print.h>

cLEDMatrix<MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT, VERTICAL_ZIGZAG_MATRIX, MATRIX_TILE_H, MATRIX_TILE_V> ledmatrix;

// cLEDMatrix creates a FastLED array inside its object and we need to retrieve
// a pointer to its first element to act as a regular FastLED array, necessary
// for NeoMatrix and other operations that may work directly on the array like FadeAll.
CRGB *leds = ledmatrix[0];

FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT,
                                                  MATRIX_TILE_H, MATRIX_TILE_V, MATRIX_FLAGS);

void led_setup()
{
   pinMode(LED_PIN, OUTPUT);

   FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, ledmatrix.Size()).setCorrection(TypicalSMD5050);
   FastLED.setBrightness(BRIGHTNESS);

   Serial.print("[LED] Matrix Size: ");
   Serial.print(MATRIX_WIDTH);
   Serial.print("x");
   Serial.println(MATRIX_HEIGHT);

   delay(200);

   matrix_clear();
   matrix->show();
}

void matrix_clear()
{
   // FastLED.clear does not work properly with multiple matrices connected via parallel inputs
   // on ESP8266 (not sure about other chips).
   memset(leds, 0, MATRIX_SIZE);
   // FastLED.clear();
}

void led_set_brightness(int brigth) {
   matrix->setBrightness(brigth);
}

void led_clear(uint32_t col) {
   matrix->fillScreen(col);
   matrix->show();
}

bool horizontal = true;
int it = 0;
int col = 0;
int timePerScan = 2000;
uint32_t bgCols[] = {
    LED_RED_HIGH,
    LED_GREEN_HIGH,
    LED_BLUE_HIGH,
};

void led_cycle_pixels()
{
   matrix->setPassThruColor();
   matrix->fillScreen(bgCols[col]);

   if (horizontal)
   {
      matrix->drawFastVLine(it, 0, MATRIX_HEIGHT, LED_WHITE_HIGH);
   }
   else
   {
      matrix->drawFastHLine(0, it, MATRIX_WIDTH, LED_WHITE_HIGH);
   }

   it++;
   matrix->show();

   auto count = horizontal ? MATRIX_WIDTH : MATRIX_HEIGHT;

   if (it >= count)
   {
      horizontal = !horizontal;
      it = 0;

      col = (col + 1) % 3;
   }

   delay(timePerScan / count);
}

void led_loop()
{
   // cycle_pixels();

}