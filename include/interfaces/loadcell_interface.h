#pragma once

#include <stdint.h>

class loadcell_interface
{
public:
    virtual bool begin(float known_weight) = 0;
    virtual float getLoad() = 0;
    virtual uint64_t getRaw() = 0;
    virtual void calibrate(float known_weight) = 0;
};