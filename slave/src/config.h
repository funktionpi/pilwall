#pragma once

#define HOSTNAME "pilwall-64x16"
#define BONJOUR_NAME "pilwall-64x16"

#ifndef RELEASE
#define SERIAL_DEBUG
#endif

#define DEBUG_PROTO 0
#define DEBUG_CMD 0
#define DEBUG_ARTNET 0
#define DEBUG_OPC 0
#define DEBUG_E131 0
#define DEBUG_TPM2 0

#define PROTO_PORT 2001
#define OPC_PORT 7890
#define ARTNET_PORT 6454
#define TPM2_IN_PORT 65506
#define TPM2_OUT_PORT 65442

#define ENABLE_ARTNET 1
#define ENABLE_E131 0
#define ENABLE_PROTO 1
#define ENABLE_OPC 0
#define ENABLE_TPM2 1

#define OPC_MAX_CLIENTS 1

#define MAX_FPS 60

#define PIN_0 4
#define PIN_1 19
#define PIN_2 33
#define PIN_3 32

// amount of amps @ 5v
#define POWER_AMPS 50

const int PINS[] = {PIN_0, PIN_1, PIN_2, PIN_3};

#define LED_CHANNEL_COUNT 4
#define DEFAULT_BRIGHTNESS 255 // 0 to 255

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

#define MAX_UDP_PACKET_SIZE 1460

// Various options for FASTLED library
// #define FASTLED_RMT_MAX_CHANNELS 4
// #define FASTLED_RMT_BUILTIN_DRIVER 1
// #define FASTLED_RMT_BUILTIN_DRIVER 1
// #define FASTLED_INTERRUPT_RETRY_COUNT 1
// #define FASTLED_ALLOW_INTERRUPTS 0
// #define FASTLED_ESP32_FLASH_LOCK 1
#define FASTLED_ESP32_I2S 1

#include <NeoPixelBus.h>
typedef NeoTiles<ColumnMajorAlternatingLayout , RowMajorLayout> Mosaic;

#define USE_FASTLED 1
#define USE_NEOPIXELBUS 0

#if USE_FASTLED
#if USE_NEOPIXELBUS
#pragma error "cannot use both fastled and neopixel bus"
#endif
#endif

const int LED_CHANNEL_WIDTH = MATRIX_SIZE / LED_CHANNEL_COUNT;