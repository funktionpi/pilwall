#include "led_controller.h"

LedController::LedController() : Framebuffer_GFX(nullptr, MATRIX_WIDTH, MATRIX_HEIGHT, NULL)
{
   // matrixWidth = MATRIX_TILE_WIDTH;
   // matrixHeight = MATRIX_TILE_HEIGHT;
   // tilesX = MATRIX_TILE_H;
   // tilesY = MATRIX_TILE_V;
   // type = NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ZIGZAG + NEO_MATRIX_COLUMNS;
   setRemapFunction([](uint16_t x, uint16_t y) -> uint16_t {
      static Mosaic mosaic(MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT, MATRIX_TILE_H, MATRIX_TILE_V);
      return mosaic.Map(x,y);
   });
}

// uint16_t LedController::width() {
//     return MATRIX_WIDTH
//     }

// uint16_t LedController::height()
// {
//    return mosaic.getHeight();
// }
