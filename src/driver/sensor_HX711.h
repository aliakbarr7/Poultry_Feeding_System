#pragma once
#include <HX711_ADC.h>
#include <EEPROM.h>
#include <interfaces/loadcell_interface.h>

// Kelas sensor HX711
class sensor_HX711 : public loadcell_interface
{
public:
    sensor_HX711(int doutPin, int sckPin);
    bool begin();
    float getLoad();
    float getWeight();
    void calibrate();

private:
    HX711_ADC LoadCell;

    static const int EEPROM_ADDRESS_CALIBRATION = 0;             // Alamat EEPROM untuk penyimpanan kalibrasi
    static const int EEPROM_ADDRESS_LAST_WEIGHT = sizeof(float); // Alamat EEPROM untuk berat terakhir

    float calibrationFactor; // Faktor kalibrasi
    float currentWeight;     // Berat saat ini
    float lastWeight = 0;    // Berat terakhir yang disimpan
    bool calibrating = false;
    float totalWeight;
    float savedWeight = 0;

    unsigned long previousMillis = 0;
    const long saveInterval = 100;    // Interval penyimpanan dalam milidetik
    unsigned long lastSaveMillis = 0; // Waktu terakhir berat disimpan
};