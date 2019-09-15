#ifndef _LED_H_
#define _LED_H_

#include <Arduino.h>
#include <FastLED.h>


// #include <internal/RgbColor.h>

#define LED_PIN         19
#define COLOR_ORDER     GRB
#define CHIPSET         WS2812B
#define BRIGHTNESS      8 // 0 to 255

// #define SATURATION      (31)

// Used by LEDMatrix
#define MATRIX_TILE_WIDTH   32 // width of EACH NEOPIXEL MATRIX (not total display)
#define MATRIX_TILE_HEIGHT  8 // height of each matrix
#define MATRIX_TILE_H       2  // number of matrices arranged horizontally
#define MATRIX_TILE_V       1  // number of matrices arranged vertically
#define MATRIX_TILE_SIZE    (MATRIX_TILE_WIDTH*MATRIX_TILE_HEIGHT)
#define MATRIX_TILE_COUNT   (MATRIX_TILE_H * MATRIX_TILE_V)

#define MATRIX_WIDTH        (MATRIX_TILE_WIDTH * MATRIX_TILE_H)
#define MATRIX_HEIGHT       (MATRIX_TILE_HEIGHT * MATRIX_TILE_V)
#define MATRIX_SIZE         (MATRIX_WIDTH*MATRIX_HEIGHT)

#define MATRIX_FLAGS        (NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG + NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS)

// This could also be defined as matrix->color(255,0,0) but those defines
// are meant to work for adafruit_gfx backends that are lacking color()
#define LED_BLACK		0

void led_clear(CRGB col = CRGB(0));
void led_draw_line(int x1, int y1, int x2, int y2, CRGB col);
void led_set_pixel(uint16_t x, uint16_t y, CRGB color);
uint16_t led_get_width();
uint16_t led_get_height();
void led_set_brightness(int brigth);
void led_clear(uint32_t col);

void led_setup();
void led_loop();

#endif