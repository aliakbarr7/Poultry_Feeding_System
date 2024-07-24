#include "network_WiFi.h"

network_WiFi::network_WiFi(const char *ssid, const char *pass) : _wifiHandler(ssid, pass) {}

bool network_WiFi::init()
{
    _wifiHandler.init();
    return WiFi.status() == WL_CONNECTED;
}

bool network_WiFi::checkStatus()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected!");
        return false;
    }
    else
    {
        Serial.println("WiFi Still connected!");
        return true;
    }
}
