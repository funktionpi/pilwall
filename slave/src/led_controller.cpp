#include "led_controller.h"

LedController::LedController()
: mosaic(MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT, MATRIX_TILE_H, MATRIX_TILE_V)
{
}

uint16_t LedController::Width()
{
   return mosaic.getWidth();
}

uint16_t LedController::Height()
{
   return mosaic.getHeight();
}

void LedController::DrawLine(int x1, int y1, int x2, int y2, CRGB col)
{
   for (size_t i = x1; i <= x2; i++)
   {
      for (int j = y1; j <= y2; j++)
      {
         SetPixel(i, j, col);
      }
   }
}