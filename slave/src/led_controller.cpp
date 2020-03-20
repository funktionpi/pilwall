#include "led_controller.h"

LedController::LedController() : Framebuffer_GFX(nullptr, MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT, NULL),
                                 mosaic(MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT, MATRIX_TILE_H, MATRIX_TILE_V)
{
}

int LedController::XY(int16_t x, int16_t y)
{
   return mosaic.Map(x,y);
}

uint16_t LedController::width()
{
   return mosaic.getWidth();
}

uint16_t LedController::height()
{
   return mosaic.getHeight();
}
