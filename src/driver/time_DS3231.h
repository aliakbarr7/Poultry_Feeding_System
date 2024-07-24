#pragma once

#include <RTClib.h>
#include <interfaces/time_interface.h>

class time_DS3231 : public time_interface
{
public:
    bool init();
    // bool setCurrentTime(uint32_t unixtime);
    // uint32_t getCurrentTime();
    time_t setCurrentTime(uint32_t unixtime);
    String getCurrentTime();
    uint32_t getHour();
    uint32_t getMinute();

private:
    RTC_DS3231 _rtc;
};