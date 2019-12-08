#pragma once

#include <Arduino.h>

class StopWatch
{
   public:
      StopWatch(bool started = true)
      {
         reset();
         if (started) start();
      }

      uint32_t elapsed() // in micro seconds
      {
         auto el = _elapsed + (_running ? (micros() - _start) : 0);
         return el;
      }

      uint32_t elapsedms()
      {
         return elapsed() / 1000;
      }

      uint32_t average()
      {
         return elapsed() / (_running ? _runs : _runs + 1);
      }

      uint32_t averagems()
      {
         return average() / 1000;
      }

      int runs()
      {
         return _runs;
      }

      void start()
      {
         ++_runs;
         _running = true;
         _start = micros();
      }

      void stop()
      {
         _elapsed += micros() - _start;
      }

      void reset()
      {
         _runs = 0;
         _running = false;
         _elapsed = 0;
      }

   private:
      uint32_t _start;
      uint32_t _elapsed;
      uint8_t _runs;
      bool _running;
};