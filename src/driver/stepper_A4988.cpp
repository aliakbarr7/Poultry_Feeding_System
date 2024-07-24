#include "stepper_A4988.h"

bool stepper_A4988::begin() {
    Serial.begin(115200);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    isRunning = false;
    digitalWrite(STEP_PIN, LOW);
    digitalWrite(DIR_PIN, LOW);
    return true;
}

void stepper_A4988::setSpeed(long rpm) {
    stepDelay = 60000000L / (STEPS_PER_REV * rpm); 
}

void stepper_A4988::stepMotor() {
    unsigned long now = micros();
    if (now - lastStepTime >= stepDelay) {
        lastStepTime = now;
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(STEP_PIN, LOW);
        currentStep++;
    }
}

void stepper_A4988::rotateClockwise() {
    currentStep = 0;
    digitalWrite(DIR_PIN, HIGH);
    Serial.println("Spinning Clockwise...");
    while (currentStep < STEPS_PER_REV) {
        if (!isRunning) {
            Serial.println("Stopped spinning Clockwise.");
            break; // Exit if stop() is called
        }
        stepMotor();
    }
}

void stepper_A4988::rotateCounterClockwise() {
    currentStep = 0;
    digitalWrite(DIR_PIN, LOW); // Arah berlawanan jarum jam
    Serial.println("Spinning Anti-Clockwise...");
    while (currentStep < STEPS_PER_REV) {
        if (!isRunning) {
            Serial.println("Stopped spinning Anti-Clockwise.");
            break; // Exit if stop() is called
        }
        stepMotor();
    }
}

bool stepper_A4988::start() {
    if (isRunning) return false;
    isRunning = true;
    rotateClockwise();
    return true;
}

bool stepper_A4988::stop() {
    if (!isRunning) return false;
    isRunning = false;
    digitalWrite(STEP_PIN, LOW);  // Set STEP pin to LOW
    digitalWrite(DIR_PIN, LOW);   // Set DIR pin to LOW
    Serial.println("Stepper stopped");
    return true;
}

void stepper_A4988::update() {
    if (isRunning) {
        stepMotor();
    }
}
