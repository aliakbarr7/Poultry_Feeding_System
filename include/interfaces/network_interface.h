#pragma once

class network_interface
{
public:
    virtual bool init() = 0;
    virtual bool checkStatus() = 0;
};
