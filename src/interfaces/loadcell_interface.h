#pragma once

#include <stdint.h>

class loadcell_interface
{
public:
    virtual bool begin() = 0;
    virtual float getLoad() = 0;
    virtual float getWeight() = 0;
    virtual void calibrate() = 0;
};