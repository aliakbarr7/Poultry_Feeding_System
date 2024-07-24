#pragma once

#include <Arduino.h>
#include <interfaces/stepper_interface.h>

class stepper_A4988 : public stepper_interface
{
public:
    stepper_A4988(int dirPin, int stepPin, int stepsPerRev)
        : DIR_PIN(dirPin), STEP_PIN(stepPin), STEPS_PER_REV(stepsPerRev), stepDelay(0), lastStepTime(0), currentStep(0), isRunning(false) {}

    bool begin();
    bool start();
    bool stop();
    void setSpeed(long rpm);
    void rotateClockwise();
    void rotateCounterClockwise();
    void update();  // Tambahkan ini

private:
    const int DIR_PIN;
    const int STEP_PIN;
    const int STEPS_PER_REV;
    long stepDelay;
    unsigned long lastStepTime;
    int currentStep;
    bool isRunning;

    void stepMotor();
};
