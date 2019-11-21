#ifndef __LOG_H___
#define __LOG_H___

#include <Arduino.h>
#include "Print.h"
#include "config.h"

#ifdef SERIAL_DEBUG
  #define LOG(...) Serial.print(__VA_ARGS__);
  #define LOGLN(...) Serial.println(__VA_ARGS__);
  #define LOGF(...) Serial.printf(__VA_ARGS__);
#else
  #define LOG(...)
  #define LOGLN(...)
  #define LOGF(...)
#endif

#endif