// #pragma once

// #include <Arduino.h>
// #include <HX711.h>
// #include <interfaces/loadcell_interface.h>

// class sensor_HX711 : public loadcell_interface
// {
// public:
//     sensor_HX711(int dout_pin, int sck_pin) : LOADCELL_DOUT_PIN(dout_pin), LOADCELL_SCK_PIN(sck_pin), scale() {}

//     bool begin(float known_weight);
//     float getLoad();
//     uint64_t getRaw();
//     void calibrate(float known_weight);

// private:
//     const int LOADCELL_DOUT_PIN;
//     const int LOADCELL_SCK_PIN;
//     HX711 scale;

// };