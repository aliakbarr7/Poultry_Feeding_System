#include "sensor_HX711.h"

sensor_HX711::sensor_HX711(int doutPin, int sckPin) : LoadCell(doutPin, sckPin), calibrationFactor(220.86), currentWeight(0) {}

bool sensor_HX711::begin()
{
    EEPROM.begin(512);
    EEPROM.get(EEPROM_ADDRESS_LAST_WEIGHT, lastWeight); // Membaca berat terakhir dari EEPROM
    EEPROM.get(EEPROM_ADDRESS_CALIBRATION, calibrationFactor);

    LoadCell.begin();
    LoadCell.start(2000, true); // Waktu stabilisasi, tare sensor

    if (LoadCell.getTareTimeoutFlag())
    {
        Serial.println("Timeout, periksa koneksi ESP32>HX711.");
        return false;
    }
    LoadCell.setCalFactor(calibrationFactor);

    Serial.print("Nilai calibration factor: ");
    Serial.println(calibrationFactor); // Tampilkan calibration factor yang dibaca dari EEPROM
    Serial.println();
    return true;
}

float sensor_HX711::getLoad()
{
    if (LoadCell.update())
    {
        if (currentWeight <= 0)
        {
            currentWeight = currentWeight + (currentWeight * -1); // Atur ke 0 jika data tidak valid
        }
    }
    else
    {
        currentWeight = LoadCell.getData();
    }
    return currentWeight / 1000; // Kembalikan berat dalam kg
}

float sensor_HX711::getWeight()
{
    float myCurrentWeight = getLoad();

    totalWeight = myCurrentWeight + lastWeight; // Hitung total berat

    // Pengambilan sampel setiap 200 ms
    unsigned long currentMillis = millis();

    if (myCurrentWeight <= -1)
    {
        totalWeight = totalWeight + myCurrentWeight; // Hitung total berat
    }

    if (myCurrentWeight >= 0)
    {
        totalWeight = myCurrentWeight + lastWeight; // Hitung total berat
    }

    // Simpan berat terakhir ke EEPROM setiap 500 ms
    if (currentMillis - lastSaveMillis >= saveInterval)
    {
        lastSaveMillis = currentMillis;                                    // Simpan waktu terakhir penyimpanan
        EEPROM.put(sensor_HX711::EEPROM_ADDRESS_LAST_WEIGHT, totalWeight); // Simpan berat saat ini ke EEPROM
        EEPROM.commit();
    }

    return totalWeight;
}

void sensor_HX711::calibrate()
{
    calibrating = true; // Set flag kalibrasi

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
            Serial.println("Zero Set Done!"); // Nol telah diset
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
            known_mass = Serial.parseFloat(); // Baca berat yang diketahui
            if (known_mass != 0)
            {
                Serial.print("Massa yang diketahui: ");
                Serial.println(known_mass);
                _resume = true;
            }
        }
    }

    LoadCell.refreshDataSet();
    float newCalibrationValue = LoadCell.getNewCalibration(known_mass); // Hitung nilai kalibrasi baru

    Serial.print("Nilai kalibrasi baru: ");
    Serial.println(newCalibrationValue);

    LoadCell.setCalFactor(newCalibrationValue);                  // Terapkan kalibrasi baru
    EEPROM.put(EEPROM_ADDRESS_CALIBRATION, newCalibrationValue); // Simpan ke EEPROM
    EEPROM.commit();

    Serial.println("Kalibrasi selesai dan diterapkan");

    // Reset lastWeight dan simpan ke EEPROM
    lastWeight = 0;                                     // Reset lastWeight menjadi 0
    EEPROM.put(EEPROM_ADDRESS_LAST_WEIGHT, lastWeight); // Simpan 0 ke EEPROM
    EEPROM.commit();

    delay(1000);         // Delay debounce
    calibrating = false; // Reset flag kalibrasi
}