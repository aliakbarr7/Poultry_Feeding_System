// #include "sensor_HX711.h"

// bool sensor_HX711::begin(float known_weight)
// {
//     scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

//     if (!scale.is_ready())
//     {
//         Serial.println("Driver HX711 tidak dapat ditemukan.");
//         return false;
//     }

//     Serial.println("HX711 ready");

//     Serial.println("Load Calibration Value from Storage...");
//     delay(1000); // Tunggu 5 detik untuk penempatan berat yang diketahui

//     long reading = scale.read_average(20); // Membaca rata-rata 20 kali
//     Serial.print("Reading: ");
//     Serial.println(reading);

//     float calibration_factor = reading / known_weight;
//     Serial.print("Load from Storagee...");
//     Serial.println(known_weight);
//     scale.set_scale(calibration_factor);

//     Serial.print("Calibration factor: ");
//     Serial.println(calibration_factor);
//     scale.tare(); // Menyetel ulang skala ke nol

//     return true;
// }

// float sensor_HX711::getLoad()
// {
//     return scale.get_units(10); // Membaca rata-rata 10 kali
// }

// uint64_t sensor_HX711::getRaw()
// {
//     return scale.read();
// }

// void sensor_HX711::calibrate(float known_weight)
// {
//     Serial.println("Calibrating...");
//     Serial.print("Set Calibration Value = ");
//     Serial.println(known_weight);
//     long reading = scale.read_average(20); // Membaca rata-rata 20 kali
//     float calibration_factor = reading / known_weight;
//     scale.set_scale(calibration_factor);

//     Serial.print("Calibration factor: ");
//     Serial.println(calibration_factor);
//     scale.tare(); // Menyetel ulang skala ke nol
// }
