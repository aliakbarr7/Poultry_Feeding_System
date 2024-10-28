#include "stepper_A4988.h"

bool stepper_A4988::begin()
{
    Serial.begin(115200);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    setSpeed(RPM_Speed);
    isRunning = false;
    clockwiseDirection = true;
    digitalWrite(STEP_PIN, LOW);
    digitalWrite(DIR_PIN, LOW);
    Serial.println("Stepper initialized.");
    return true;
}

void stepper_A4988::setSpeed(long rpm)
{
    stepDelay = 60000000L / (STEPS_PER_REV * rpm); // Mengatur kecepatan stepper
    Serial.print(rpm);
}

void stepper_A4988::stepMotor()
{
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(10); // Pulsa pendek ke STEP_PIN
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelay - 10);
}

void stepper_A4988::rotateCounterClockwise()
{
    digitalWrite(DIR_PIN, LOW); // Mengatur arah putaran searah jarum jam
    Serial.println("Spinning Clockwise...");
}

// Fungsi untuk memutar motor berlawanan arah selama 5 detik
void stepper_A4988::rotateClockwise()
{
    isRunning = true; // Atur isRunning ke true agar motor bisa berputar
    Serial.println("Memutar berlawanan jarum jam selama 5 detik...");
    unsigned long startTime = millis();
    digitalWrite(DIR_PIN, HIGH); // Atur arah ke berlawanan jarum jam

    while (millis() - startTime < 1500)
    {
        stepMotor(); // Jalankan motor
    }

    isRunning = false; // Setelah 3 detik, hentikan motor
}

void stepper_A4988::start()
{
    if (!isRunning)
    {
        isRunning = true;
        if (clockwiseDirection)
        {
            rotateCounterClockwise();

            // Dummy Tes LED
            // digitalWrite(DIR_PIN, HIGH);
        }
    }
    else
    {
        Serial.println("Motor sudah berjalan.");
    }
}

void stepper_A4988::stop()
{
    if (isRunning)
    {
        isRunning = false;
        digitalWrite(STEP_PIN, LOW); // Matikan sinyal STEP
        Serial.println("Motor berhenti.");

        // Berhenti selama 1 detik
        delay(1500);

        // Memutar berlawanan jarum jam selama 5 detik
        rotateClockwise();

        // Setelah 2 detik, motor berhenti total
        digitalWrite(STEP_PIN, LOW); // Matikan STEP pin
        digitalWrite(DIR_PIN, LOW);  // Matikan DIR pin
        Serial.println("Stepper berhenti total.");

        // Dummy Tes LED
        // digitalWrite(DIR_PIN, LOW);
    }
    else
    {
        Serial.println("Motor sudah berhenti.");
    }
}

void stepper_A4988::update()
{
    // Memanggil stepMotor() jika isRunning bernilai true
    if (isRunning)
    {
        stepMotor();
        // Serial.print("State isRunning = ");
        // Serial.println(isRunning);
    }
}
