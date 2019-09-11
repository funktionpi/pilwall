#include "led.h"

#include <Print.h>

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>

RgbColor red(SATURATION, 0, 0);
RgbColor green(0, SATURATION, 0);
RgbColor blue(0, 0, SATURATION);
RgbColor white(SATURATION);
RgbColor black(0);

HslColor hslRed(red);
HslColor hslGreen(green);
HslColor hslBlue(blue);
HslColor hslWhite(white);
HslColor hslBlack(black);

NeoMosaic <ColumnMajorAlternatingLayout> mosaic(
    MATRIX_TILE_WIDTH,
    MATRIX_TILE_HEIGHT,
    MATRIX_TILE_H,
    MATRIX_TILE_V);

NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip(MATRIX_SIZE, LED_PIN);

void led_setup()
{
   pinMode(LED_PIN, OUTPUT);

   Serial.print("[LED] Matrix Size: ");
   Serial.print(MATRIX_WIDTH);
   Serial.print("x");
   Serial.println(MATRIX_HEIGHT);


   strip.SetBrightness(BRIGHTNESS);
   strip.Begin();
   led_clear();
   strip.Show();
   Serial.println("[LED] setup done");
}

void led_set_brightness(int brigth) {
   strip.SetBrightness(brigth);
}

void led_clear(RgbColor col) {
   strip.ClearTo(col);
}

void draw_line(int x1, int y1, int x2, int y2, RgbColor col)
{
   Serial.printf("[LED] draw line between (%d,%d) and (%d,%d)\n", x1, y1, x2, y2);

   for (size_t i = x1; i <= x2; i++)
   {
      for (int j = y1; j <= y2; j++)
      {
         auto id = mosaic.Map(i,j);
         // Serial.printf("[LED] setting color for (%d,%d), ledid = %d\n", i, j, id);
         strip.SetPixelColor(id, col);
      }
   }

}

bool horizontal = true;
int it = 0;
int col = 0;
RgbColor bgCols[] = { red, green, blue };

void led_cycle_pixels()
{
   strip.ClearTo(bgCols[col]);

   auto count = horizontal ? MATRIX_WIDTH : MATRIX_HEIGHT;

   Serial.printf("[LED] line %d / %d\n", it, count);

   if (horizontal)
   {
      draw_line(it, 0, it, MATRIX_HEIGHT, white);
   }
   else
   {
      draw_line(0, it, MATRIX_WIDTH, it, white);
   }

   it++;

   if (it >= count)
   {
      horizontal = !horizontal;
      it = 0;

      col = (col + 1) % 3;
      Serial.printf("[LED] now clearing to 0x%4x\n", bgCols[col]);
   }
}

void led_loop()
{
   // led_cycle_pixels();
   strip.Show();
   // delay(500);

}