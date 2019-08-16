#include "led.h"

#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>
#include <LEDMatrix.h>

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

  Serial.print("Matrix Size: ");
  Serial.print(MATRIX_WIDTH);
  Serial.print(" ");
  Serial.println(MATRIX_HEIGHT);

  delay(500);

  matrix_clear();
  matrix->show();

  // matrix->fillScreen(LED_RED_HIGH);
  // matrix->drawFastVLine(0,0,MATRIX_HEIGHT-1, LED_BLUE_HIGH);
  // matrix->drawFastVLine(MATRIX_WIDTH-1,0,MATRIX_HEIGHT-1, LED_CYAN_HIGH);
  // matrix->drawFastHLine(0,0,MATRIX_WIDTH-1, LED_GREEN_HIGH);
  // matrix->drawFastHLine(0,MATRIX_HEIGHT-1,MATRIX_WIDTH-1, LED_PURPLE_HIGH);
  // // matrix->show();
  // FastLED.show();
}

void matrix_clear()
{
  // FastLED.clear does not work properly with multiple matrices connected via parallel inputs
  // on ESP8266 (not sure about other chips).
  memset(leds, 0, MATRIX_SIZE);
  // FastLED.clear();
}

// In a case of a tile of neomatrices, this test is helpful to make sure that the
// pixels are all in sequence (to check your wiring order and the tile options you
// gave to the constructor).
void count_pixels()
{
  auto mw = MATRIX_WIDTH;
  auto mh = MATRIX_HEIGHT;

  matrix_clear();
  for (uint16_t i = 0; i < MATRIX_SIZE; i++)
  {
    for (uint16_t j = 0; j < mw; j++)
    {
      auto color = i % 3 == 0 ? LED_BLUE_HIGH : i % 3 == 1 ? LED_RED_HIGH : LED_GREEN_HIGH;
      matrix->drawPixel(j, i, (uint32_t)color);
// depending on the matrix size, it's too slow to display each pixel, so
// make the scan init faster. This will however be too fast on a small matrix.
#ifdef ESP8266
      if (!(j % 3))
        matrix->show();
      yield(); // reset watchdog timer
#elif ESP32
      delay(1);
      matrix->show();
#else
      matrix->show();
#endif
    }
  }
}

void display_lines()
{
  matrix_clear();

  auto mw = MATRIX_WIDTH;
  auto mh = MATRIX_HEIGHT;

  // 4 levels of crossing red lines.
  matrix->drawLine(0, mh / 2 - 2, mw - 1, 2, LED_RED_VERYLOW);
  matrix->drawLine(0, mh / 2 - 1, mw - 1, 3, LED_RED_LOW);
  matrix->drawLine(0, mh / 2, mw - 1, mh / 2, LED_RED_MEDIUM);
  matrix->drawLine(0, mh / 2 + 1, mw - 1, mh / 2 + 1, LED_RED_HIGH);

  // 4 levels of crossing green lines.
  matrix->drawLine(mw / 2 - 2, 0, mw / 2 - 2, mh - 1, LED_GREEN_VERYLOW);
  matrix->drawLine(mw / 2 - 1, 0, mw / 2 - 1, mh - 1, LED_GREEN_LOW);
  matrix->drawLine(mw / 2 + 0, 0, mw / 2 + 0, mh - 1, LED_GREEN_MEDIUM);
  matrix->drawLine(mw / 2 + 1, 0, mw / 2 + 1, mh - 1, LED_GREEN_HIGH);

  // Diagonal blue line.
  matrix->drawLine(0, 0, mw - 1, mh - 1, LED_BLUE_HIGH);
  matrix->drawLine(0, mh - 1, mw - 1, 0, LED_ORANGE_MEDIUM);
  matrix->show();
}

void display_boxes()
{
  auto mw = MATRIX_WIDTH;
  auto mh = MATRIX_HEIGHT;

  matrix_clear();
  matrix->drawRect(0, 0, mw, mh, LED_BLUE_HIGH);
  matrix->drawRect(1, 1, mw - 2, mh - 2, LED_GREEN_MEDIUM);
  matrix->fillRect(2, 2, mw - 4, mh - 4, LED_RED_HIGH);
  matrix->fillRect(3, 3, mw - 6, mh - 6, LED_ORANGE_MEDIUM);
  matrix->show();
}

int i = 0;

void led_loop()
{
  // count_pixels();
  display_boxes();
  delay(1000);
  display_lines();
  delay(1000);

  // EVERY_N_MILLIS(5) {
    // Serial.print("draw line ");
    // Serial.print(i);
    // Serial.println();

  //   auto red = Framebuffer_GFX::CRGBtoint32(CRGB::Red);
  //   auto green = Framebuffer_GFX::CRGBtoint32(CRGB::Green);

  //   matrix->fillScreen(LED_RED_HIGH);
  //   // matrix->drawRect();
  //   matrix->drawFastVLine(i, 0, MATRIX_HEIGHT-1, LED_GREEN_HIGH);
  //   // matrix->drawCircle(4, 4, 2, CRGB(255,255,0));

  //   matrix->show();
  //   i++;
  //   i %= MATRIX_WIDTH;
  // }
}