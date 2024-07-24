#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WifiHandler.h>
#include <interfaces/network_interface.h>

class network_WiFi : public network_interface
{

public:
    network_WiFi(const char *ssid, const char *pass);
    bool init();
    bool checkStatus();

private:
    WifiHandler _wifiHandler;
};
