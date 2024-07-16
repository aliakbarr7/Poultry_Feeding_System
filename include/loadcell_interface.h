#pragma once

#include <stdint.h>

class loadcell_interface
{
    public:
        virtual bool begin() = 0;
        virtual float getLoad() = 0;
        virtual uint64_t getRaw() = 0;
};