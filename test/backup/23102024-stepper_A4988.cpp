// #include "stepper_A4988.h"

// bool stepper_A4988::begin()
// {
//     Serial.begin(115200);
//     pinMode(STEP_PIN, OUTPUT);
//     pinMode(DIR_PIN, OUTPUT);
//     setSpeed(RPM_Speed);
//     isRunning = false;
//     digitalWrite(STEP_PIN, LOW);
//     digitalWrite(DIR_PIN, LOW);
//     Serial.println("Stepper initialized.");
//     return true;
// }

// void stepper_A4988::setSpeed(long rpm)
// {
//     stepDelay = 60000000L / (STEPS_PER_REV * rpm); // Mengatur kecepatan stepper
//     Serial.print("Speed set to ");
//     Serial.print(rpm);
//     Serial.println(" RPM.");
// }

// void stepper_A4988::stepMotor()
// {
//     unsigned long now = micros();  // Menggunakan micros() untuk interval yang lebih akurat
//     if (now - lastStepTime >= stepDelay)
//     {
//         lastStepTime = now;
//         digitalWrite(STEP_PIN, HIGH);
//         delayMicroseconds(10);  // Pulsa pendek ke STEP_PIN
//         digitalWrite(STEP_PIN, LOW);
//         delayMicroseconds(10);  // Jeda singkat setelah STEP
//     }
// }

// void stepper_A4988::rotateClockwise()
// {
//     digitalWrite(DIR_PIN, HIGH); // Mengatur arah putaran searah jarum jam
//     Serial.println("Spinning Clockwise...");
//     // Tidak ada loop while(isRunning) di sini
//     // Biarkan fungsi update() yang mengontrol pemanggilan stepMotor()
// }

// void stepper_A4988::rotateCounterClockwise()
// {
//     digitalWrite(DIR_PIN, LOW);  // Mengatur arah berlawanan jarum jam
//     Serial.println("Spinning Anti-Clockwise...");
//     // Tidak ada loop while(isRunning) di sini
//     // Biarkan fungsi update() yang mengontrol pemanggilan stepMotor()
// }

// bool stepper_A4988::start()
// {
//     if (isRunning)
//         return false;
//     isRunning = true;
//     Serial.println("Motor started (Clockwise), isRunning = true");
//     rotateClockwise();  // Memulai pergerakan searah jarum jam
//     return true;
// }

// bool stepper_A4988::stop()
// {
//     if (!isRunning)
//         return false;

//     isRunning = false;
//     Serial.println("Motor stopped, isRunning = false");
//     digitalWrite(STEP_PIN, LOW);  // Matikan sinyal STEP
//     digitalWrite(DIR_PIN, LOW);   // Matikan sinyal DIR
//     Serial.println("Stepper berhenti selama 1 detik");

//     // Berhenti selama 1 detik
//     delay(1000);

//     // Putar berlawanan jarum jam selama 5 detik
//     Serial.println("Berputar berlawanan jarum jam selama 5 detik...");
//     unsigned long startTime = millis();
//     isRunning = true;  // Atur isRunning ke true agar motor bisa bergerak
//     Serial.println("Motor started (CounterClockwise), isRunning = true");
//     while (millis() - startTime < 5000)
//     {
//         rotateCounterClockwise(); // Panggil rotateCounterClockwise untuk langkah-langkah
//     }

//     isRunning = false;
//     Serial.println("Motor stopped after 5 seconds, isRunning = false");

//     // Berhenti total
//     digitalWrite(STEP_PIN, LOW); // Pastikan STEP pin LOW
//     digitalWrite(DIR_PIN, LOW);  // Pastikan DIR pin LOW
//     Serial.println("Stepper berhenti total.");

//     return true;
// }

// void stepper_A4988::update()
// {
//     if (isRunning)
//     {
//         stepMotor();
//         Serial.println("isRunning = true (Update)");  // Debug: Lihat kondisi isRunning saat update
//     }
//     else
//     {
//         Serial.println("isRunning = false (Update)");  // Debug: Lihat kondisi isRunning saat update
//     }
// }
