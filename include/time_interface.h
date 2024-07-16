#pragma once

#include <stdint.h>
#include <string>

class time_interface
{
    public:
        virtual bool begin() = 0;
        virtual time_t getCurrentTime() = 0;
};