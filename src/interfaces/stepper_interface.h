#pragma once

class stepper_interface
{
public:
    virtual bool begin() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void setSpeed(long rpm) = 0;    
    virtual void stepMotor() = 0;    
    virtual void rotateClockwise() = 0;     
    virtual void rotateCounterClockwise() = 0;
    virtual void update() = 0;
};