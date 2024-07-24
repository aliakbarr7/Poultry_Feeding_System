#include "display_PCF8574.h"

bool display_PCF8574::begin()
{
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Chicken Fedeer");
    delay(2000);
    lastRefreshTime = millis();
    refreshDisplay();
    return true;
}

bool display_PCF8574::setLoadValue(float loadValue)
{
    _loadValue = loadValue;
    refreshDisplay();
    return true;
}

bool display_PCF8574::setCalibration(float known_weight)
{
    _calibrationWeight = known_weight;
    calibrationActive = true;
    refreshDisplay();
    calibrationActive = false;
    return true;
}

bool display_PCF8574::setFeedStatus(std::string FeedStatusStr)
{
    _feedStatus = FeedStatusStr;
    refreshDisplay();
    return true;
}

bool display_PCF8574::setFeedPerDay(float setFeedPerDay)
{
    _feedPerDay = setFeedPerDay;
    refreshDisplay();
    return true;
}

bool display_PCF8574::setSchedule1(std::string timeSchedule1)
{
    _schedule1 = timeSchedule1;
    refreshDisplay();
    return true;
}

bool display_PCF8574::setSchedule2(std::string timeSchedule2)
{
    _schedule2 = timeSchedule2;
    refreshDisplay();
    return true;
}

bool display_PCF8574::setSchedule3(std::string timeSchedule3)
{
    _schedule3 = timeSchedule3;
    refreshDisplay();
    return true;
}

bool display_PCF8574::setDateTime(std::string dateTimeStr)
{
    _dateTimeStr = dateTimeStr;
    refreshDisplay();
    return true;
}

bool display_PCF8574::setWifiConn(bool connected)
{
    _wifiStatus = connected;
    refreshDisplay();
    return true;
}

bool display_PCF8574::showMessage(std::string notifMessage1, std::string notifMessage2)
{
    _notifMessage1 = notifMessage1;
    _notifMessage2 = notifMessage2;
    messageActive = true;
    messageStartTime = millis(); // Catat waktu mulai pesan
    refreshDisplay();
    return true;
}

void display_PCF8574::refreshDisplay()
{
    unsigned long currentTime = millis();

    if (messageActive)
    {
        if (currentTime - messageStartTime >= messageDuration)
        {
            messageActive = false;
            lastRefreshTime = currentTime;
            refreshDisplay(); // Kembali ke tampilan normal setelah pesan selesai
        }
        else
        {
            displayNotif(); // Tampilkan pesan selama durasi pesan
            return;
        }
    }

    if (calibrationActive)
    {
        displayCalibration();
    }
    else
    {
        if (currentTime - lastRefreshTime >= 2000)
        {
            currentScreen = (currentScreen + 1) % 4;
            lastRefreshTime = currentTime;
        }

        switch (currentScreen)
        {
        case 0:
            displayScreen1();
            break;
        case 1:
            displayScreen2();
            break;
        case 2:
            displayScreen3();
            break;
        case 3:
            displayScreen4();
            break;
        }
    }
}

void display_PCF8574::displayScreen1()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Pakan: %.2f Kg", _loadValue);
    lcd.setCursor(0, 1);
    lcd.printf("P/Hari: %.2f Kg", _feedPerDay);
}

void display_PCF8574::displayScreen2()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf(_dateTimeStr.c_str());
    lcd.setCursor(0, 1);
    lcd.printf("Jadwal_1: %s", _schedule1.c_str());
}

void display_PCF8574::displayScreen3()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Jadwal_2: %s", _schedule2.c_str());
    lcd.setCursor(0, 1);
    lcd.printf("Jadwal_3: %s", _schedule3.c_str());
}

void display_PCF8574::displayScreen4()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("F_Sts: %s", _feedStatus.c_str());
}

void display_PCF8574::displayCalibration()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kalibrasi...");
    lcd.setCursor(0, 1);
    lcd.printf("N_FK: %.2f", _calibrationWeight);
}

void display_PCF8574::displayNotif()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(_notifMessage1.c_str());
    lcd.setCursor(0, 1);
    lcd.print(_notifMessage2.c_str());
}

bool display_PCF8574::update()
{
    unsigned long currentTime = millis();

    if (messageActive)
    {
        if (currentTime - messageStartTime >= messageDuration)
        {
            messageActive = false;
            lastRefreshTime = currentTime;
            refreshDisplay(); // Kembali ke tampilan normal setelah pesan selesai
        }
    }
    return messageActive;
}