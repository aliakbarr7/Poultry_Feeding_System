// #pragma once

// #include <RTClib.h>
// #include <WiFi.h>
// #include <WiFiUdp.h>
// #include <NTPClient.h>
// #include <interfaces/time_interface.h>

// class time_DS3231 : public time_interface
// {
// public:
//     bool init();
//     // bool setCurrentTime(uint32_t unixtime);
//     // uint32_t getCurrentTime();
//     time_t setCurrentTime(uint32_t unixtime);
//     String getCurrentTime();
//     uint32_t getHour();
//     uint32_t getMinute();
//     bool setTimeFromNTP();

// private:
//     RTC_DS3231 _rtc;
//     bool timeSet = false; // Flag untuk memeriksa apakah waktu sudah diatur
//     WiFiUDP ntpUDP;
//     NTPClient timeClient = NTPClient(ntpUDP, "pool.ntp.org", 28800);
//     const long utcOffsetInSeconds = 28800; // Offset untuk WITA (Palu, Sulawesi Tengah)
// };