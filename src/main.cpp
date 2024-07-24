#include <Arduino.h>
#include <dev_config.h>
#include <SPI.h>

#include "FeedingApp.h"
#include <driver/communication_mqtt.h>
#include <driver/display_PCF8574.h>
#include <driver/network_WiFi.h>
#include <driver/sensor_HX711.h>
#include <driver/stepper_A4988.h>
#include <driver/storage_LittleFS.h>
#include <driver/time_DS3231.h>


// Inisialisasi objek
storage_LittleFS storage;
sensor_HX711 loadcell(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
network_WiFi network(WIFI_SSID, WIFI_PASSWORD);
communication_mqtt comm_mqtt(MQTT_SERVER, MQTT_CLIENTID);
display_PCF8574 display;
stepper_A4988 stepper(DIR, STEP, SPR);
time_DS3231 timeHandler;

FeedingApp feedingApp(network, comm_mqtt, display, loadcell, stepper, storage, timeHandler);

void setup()
{
    feedingApp.init();
}

void loop()
{
    feedingApp.loop();
}