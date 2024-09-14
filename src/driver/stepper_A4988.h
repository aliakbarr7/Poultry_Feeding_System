#pragma once

#include <Arduino.h>
#include <interfaces/stepper_interface.h>

class stepper_A4988 : public stepper_interface
{
public:
    stepper_A4988(int dirPin, int stepPin, int stepsPerRev, int rpm)
        : DIR_PIN(dirPin), STEP_PIN(stepPin), STEPS_PER_REV(stepsPerRev), RPM_Speed(rpm), stepDelay(0), lastStepTime(0), currentStep(0), isRunning(false) {}

    bool begin();
    void start();
    void stop();
    void setSpeed(long rpm);
    void stepMotor();
    void rotateClockwise();
    void rotateCounterClockwise();
    void update();  // Tambahkan ini

private:
    const int DIR_PIN;
    const int STEP_PIN;
    const int STEPS_PER_REV;
    const int RPM_Speed;
    long stepDelay;
    unsigned long lastStepTime;
    
    int currentStep;
    bool isRunning;
    bool clockwiseDirection;

    
};
