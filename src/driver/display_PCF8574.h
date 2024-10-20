#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string>
#include "interfaces/display_interface.h"

class display_PCF8574 : public display_interface
{
public:
    display_PCF8574() : lcd(0x27, 16, 2), currentScreen(0), calibrationActive(false), messageActive(false) {} // Ganti alamat I2C jika diperlukan
    bool begin();
    bool setLoadValue(float loadValue);
    bool setCalibration();
    bool setFeedStatus(std::string FeedStatusStr);
    bool setFeedPerDay(float setFeedPerDay);
    bool setSchedule1(std::string timeSchedule1);
    bool setSchedule2(std::string timeSchedule2);
    bool setSchedule3(std::string timeSchedule3);
    bool setDateTime(std::string dateTimeStr);
    bool setWifiConn(bool connected);
    bool showMessage(std::string notifMessage1, std::string notifMessage2);
    bool setAPConfig();
    bool update();

private:
    LiquidCrystal_I2C lcd;
    float _loadValue = 0;
    std::string _feedStatus;
    float _feedPerDay;
    std::string _schedule1;
    std::string _schedule2;
    std::string _schedule3;
    std::string _dateTimeStr;
    std::string _notifMessage1;
    std::string _notifMessage2;
    bool _wifiStatus = false;

    int currentScreen;
    bool calibrationActive;
    bool APConfigActive;
    bool messageActive;
    unsigned long lastRefreshTime;
    unsigned long messageStartTime;
    const unsigned long refreshInterval = 2000; // Interval untuk beralih layar
    const unsigned long messageDuration = 2000; // Durasi pesan ditampilkan

    void refreshDisplay();
    void displayScreen1();
    void displayScreen2();
    void displayScreen3();
    void displayScreen4();
    void displayAPConfig();
    void displayCalibration();
    void displayNotif();
    void animateTransition(); // Tambahkan ini
};
