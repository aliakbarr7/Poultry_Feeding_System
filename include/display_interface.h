#pragma once

#include <string>

class display_interface
{
    public:
        virtual bool begin() = 0;
        virtual bool setLoadValue(float loadValue) = 0;
        virtual bool setDateTime(std::string dateTimeStr) = 0;
        virtual bool setWifiConn(bool connected) = 0;
};