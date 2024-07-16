#include <Arduino.h>

#include <WifiHandler.h>

WifiHandler wifi("your ssid", "password");

void setup()
{
  wifi.init();
}

void loop()
{
}