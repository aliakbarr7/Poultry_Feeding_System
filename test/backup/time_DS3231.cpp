// #include "time_DS3231.h"

// bool time_DS3231::init()
// {
//     // Inisialisasi RTC
//     _rtc.begin();

//     if (!_rtc.begin())
//     {
//         Serial.println("Couldn't find RTC");
//         return false;
//     }

//     // Periksa apakah RTC kehilangan daya
//     if (_rtc.lostPower())
//     {
//         Serial.println("RTC lost power, let's set the time!");
//         // Set RTC ke waktu kompilasi atau waktu default lainnya
//         _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//     }
    
//     // Inisialisasi NTP
//     timeClient = NTPClient(ntpUDP);
//     timeClient.begin();
//     timeClient.setTimeOffset(utcOffsetInSeconds);

//     return true;
// }

// time_t time_DS3231::setCurrentTime(uint32_t unixtime)
// {
//     // _rtc.adjust(DateTime(unixtime));
//     _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

//     return true;
// }

// String time_DS3231::getCurrentTime()
// {
//     // Serial.println(_rtc.now().unixtime());
//     const char *namaHari[] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
//     // Mendapatkan hari dalam seminggu (0 = Minggu, 1 = Senin, dst.)
//     uint8_t hari = _rtc.now().dayOfTheWeek();

//     return String(_rtc.now().day(), DEC) + "/" + String(_rtc.now().month(), DEC) + "/" + String(_rtc.now().year(), DEC) + "-" + String(_rtc.now().hour(), DEC) + ":" + String(_rtc.now().minute(), DEC);
// }

// uint32_t time_DS3231::getHour()
// {
//     return _rtc.now().hour();
// }

// uint32_t time_DS3231::getMinute()
// {
//     return _rtc.now().minute();
// }

// bool time_DS3231::setTimeFromNTP()
// {
//     if (!timeSet) // Hanya set waktu jika belum diatur
//     {
//         timeClient.update(); // Memperbarui waktu dari NTP
//         if (timeClient.getEpochTime() > 0) // Memastikan waktu yang valid
//         {
//             _rtc.adjust(DateTime(timeClient.getEpochTime())); // Atur RTC dengan waktu NTP
//             timeSet = true; // Tandai bahwa waktu telah diatur
//             Serial.println("RTC time set from NTP.");
//             return true;
//         }
//         else
//         {
//             Serial.println("Failed to get time from NTP.");
//             return false;
//         }
//     }
//     return false; // Waktu sudah diatur sebelumnya
// }