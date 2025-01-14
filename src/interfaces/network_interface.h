#pragma once

class network_interface
{
public:
    virtual bool init() = 0;
    virtual bool checkStatus() = 0;
    virtual bool WiFiConfig() = 0;
    virtual bool disconnect() = 0;
};
