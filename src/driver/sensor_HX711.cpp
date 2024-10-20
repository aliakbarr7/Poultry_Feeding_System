#include "sensor_HX711.h"

// Konstruktor untuk sensor_HX711
sensor_HX711::sensor_HX711(int doutPin, int sckPin) : LoadCell(doutPin, sckPin), calibrationFactor(220.86), currentWeight(0) {}

// Implementasi metode 'begin' untuk inisialisasi load cell
bool sensor_HX711::begin()
{
    EEPROM.begin(512);
    EEPROM.get(EEPROM_ADDRESS, calibrationFactor);
    LoadCell.begin();
    LoadCell.start(2000, true); // Stabilizing time, tare sensor
    if (LoadCell.getTareTimeoutFlag())
    {
        Serial.println("Timeout, periksa koneksi ESP32>HX711.");
        return false;
    }
    LoadCell.setCalFactor(calibrationFactor);
    return true;
}

// Implementasi metode 'getLoad' untuk mendapatkan berat saat ini
float sensor_HX711::getLoad()
{
    if (LoadCell.update())
    {
        currentWeight = LoadCell.getData();
        if (currentWeight <= 1)
        {
            currentWeight = 0;
        }
    }
    return currentWeight / 1000; // Berat dalam kg
}

// Implementasi metode 'calibrate' untuk mengkalibrasi load cell
void sensor_HX711::calibrate()
{
    Serial.println("***");
    Serial.println("Mulai kalibrasi:");
    Serial.println("Tempatkan load cell pada permukaan yang datar.");
    Serial.println("Saat ini pastikan tidak ada benda di atas loadcell.");
    Serial.println("Kirim 't' dari serial monitor untuk mengatur kalibrasi.");

    boolean _resume = false;
    while (_resume == false)
    {
        LoadCell.update();
        if (Serial.available() > 0)
        {
            char inByte = Serial.read();
            if (inByte == 't')
                LoadCell.tareNoDelay(); // Set zero (tare) untuk load cell
        }
        if (LoadCell.getTareStatus() == true)
        {
            Serial.println("Zero Set Done!"); // Zero sudah disetel
            _resume = true;
        }
    }

    Serial.println("Tempatkan benda dengan massa yang diketahui di atas loadcell.");
    Serial.println("Kirim berat dari massa ini (contoh: 100.0) dari serial monitor. Dalam satuan 'Gram'.");

    float known_mass = 0;
    _resume = false;
    while (_resume == false)
    {
        LoadCell.update();
        if (Serial.available() > 0)
        {
            known_mass = Serial.parseFloat(); // Membaca berat yang diketahui
            if (known_mass != 0)
            {
                Serial.print("Massa yang diketahui: ");
                Serial.println(known_mass);
                _resume = true;
            }
        }
    }

    // Refresh dataset untuk memastikan pembacaan massa yang benar
    LoadCell.refreshDataSet();                                          
    float newCalibrationValue = LoadCell.getNewCalibration(known_mass); // Hitung nilai kalibrasi baru

    Serial.print("Nilai kalibrasi baru: ");
    Serial.println(newCalibrationValue);

    // Simpan nilai kalibrasi baru ke EEPROM dan langsung digunakan
    LoadCell.setCalFactor(newCalibrationValue);              // Terapkan kalibrasi baru
    EEPROM.put(EEPROM_ADDRESS, newCalibrationValue);          // Simpan di EEPROM
    EEPROM.commit();

    Serial.println("Kalibrasi selesai dan diterapkan");
}
