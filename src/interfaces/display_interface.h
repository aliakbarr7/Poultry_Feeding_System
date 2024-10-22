#pragma once

#include <string>

class display_interface
{
    public:
        virtual bool begin() = 0;
        virtual bool setLoadValue(float loadValue) = 0;
        virtual bool setCalibration() = 0;
        virtual bool setFeedStatus(std::string FeedStatusStr) = 0;
        virtual bool setFeedPerDay(float setFeedPerDay) = 0;
        virtual bool setSchedule1(std::string timeSchedule1) = 0;
        virtual bool setSchedule2(std::string timeSchedule2) = 0;
        virtual bool setSchedule3(std::string timeSchedule3) = 0;
        virtual bool setDateTime(std::string dateTimeStr) = 0;
        virtual bool setWifiConn(bool connected) = 0;
        virtual bool showMessage(std::string notifMessage1, std::string notifMessage2) = 0;
        virtual bool setAPConfig() = 0;
        virtual bool update() = 0;
};