#ifndef __CONFIG_H__
#define __CONFIG_H__

#define HOSTNAME "esp32-01"
#define BONJOUR_NAME "ledslave"

#define SERIAL_DEBUG
// #define DEBUG_WEBSOCKET
// #define DEBUG_UDP
// #define DEBUG_RAW
// #define DEBUG_TCP
// #define DEBUG_CMD

#define RAW_PORT 1001
#define UDP_PORT 2001
#define TCP_PORT 3001
#define WEBSOCKET_PORT 9090

#define PIN_0 4
#define PIN_1 19
#define PIN_2 33
#define PIN_3 32

const int PINS[] = {PIN_0, PIN_1, PIN_2, PIN_3};

#define LED_CHANNEL_COUNT 4
#define DEFAULT_BRIGHTNESS 32 // 0 to 255

// Used by LEDMatrix
#define MATRIX_TILE_WIDTH 32 // width of EACH NEOPIXEL MATRIX (not total display)
#define MATRIX_TILE_HEIGHT 8 // height of each matrix
#define MATRIX_TILE_H 2      // number of matrices arranged horizontally
#define MATRIX_TILE_V 2      // number of matrices arranged vertically
#define MATRIX_TILE_SIZE (MATRIX_TILE_WIDTH * MATRIX_TILE_HEIGHT)
#define MATRIX_TILE_COUNT (MATRIX_TILE_H * MATRIX_TILE_V)

#define MATRIX_WIDTH (MATRIX_TILE_WIDTH * MATRIX_TILE_H)
#define MATRIX_HEIGHT (MATRIX_TILE_HEIGHT * MATRIX_TILE_V)
#define MATRIX_SIZE (MATRIX_WIDTH * MATRIX_HEIGHT)

const int LED_CHANNEL_WIDTH = MATRIX_SIZE / LED_CHANNEL_COUNT;

#endif