#pragma once
#include <HX711_ADC.h>
#include <EEPROM.h>
#include <interfaces/loadcell_interface.h>


class sensor_HX711 : public loadcell_interface
{
public:
    sensor_HX711(int doutPin, int sckPin);
    bool begin();
    float getLoad();
    void calibrate();

private:
    HX711_ADC LoadCell;
    float calibrationFactor;      // Faktor kalibrasi
    float currentWeight;          // Berat saat ini
    const int EEPROM_ADDRESS = 0; // Alamat EEPROM untuk penyimpanan kalibrasi
};
