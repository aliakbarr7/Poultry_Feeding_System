#pragma once

#include <stdint.h>
#include <string>

class time_interface
{
public:
    virtual bool init() = 0;
    virtual time_t setCurrentTime(uint32_t unixtime) = 0;
    virtual String getCurrentTime() = 0;
    virtual uint32_t getHour() = 0;
    virtual uint32_t getMinute() = 0;
    virtual bool setTimeFromNTP() = 0;
};