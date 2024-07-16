#pragma once

class stepper_interface
{
    public:
        virtual bool begin() = 0;
        virtual bool start() = 0;
        virtual bool stop() = 0;
};