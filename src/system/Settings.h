#pragma once

#if __has_include("Arduino.h")
  #include "Arduino.h"
#else
  #define log_w(...)
  #define log_i(...)
  #define log_e(...)
  #define log_d(...)
#endif

#include <ArduinoJson.h>
#include <storage_interface.h>

class Settings {

public:
  Settings(storage_interface &stg);

  bool load(std::string filename = "/settings.json");
  bool store(std::string filename = "/settings.json");

  bool setInterval(uint32_t milliseconds);
  bool setTimezone(int8_t timezone);

  uint32_t getInterval();
  int8_t getTimezone();

private:
  storage_interface &_stg;

  int8_t _timezone = 7;
  uint32_t _updateInterval_ms = 1000;
};