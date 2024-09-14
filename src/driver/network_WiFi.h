#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <interfaces/network_interface.h>


class network_WiFi : public network_interface
{

public:
    network_WiFi();
    bool init();
    bool checkStatus();
    bool WiFiConfig();

private:
    WiFiManager wifiManager;
    Preferences preferences;
    bool wifiWasConnected = false; // Variabel untuk mendeteksi perubahan status WiFi
};
