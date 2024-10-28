#include <dev_config.h>
#include "network_WiFi.h"


network_WiFi::network_WiFi()
{
    wifiWasConnected = false;
}

bool network_WiFi::init()
{
    // Coba untuk terhubung ke jaringan WiFi yang ada
    WiFi.begin();
    if (WiFi.waitForConnectResult() == WL_CONNECTED)
    {
        wifiWasConnected = true;
    }
    else
    {
        wifiWasConnected = false;
    }
    return WiFi.status() == WL_CONNECTED;
}

bool network_WiFi::checkStatus()
{
    // Memeriksa apakah WiFi masih terhubung atau terputus
    if (WiFi.status() != WL_CONNECTED && wifiWasConnected)
    {
        wifiWasConnected = false;
        WiFi.reconnect(); // Mencoba menghubungkan kembali ke WiFi
    }

    // Jika WiFi terdeteksi kembali setelah terputus
    if (WiFi.status() == WL_CONNECTED && !wifiWasConnected)
    {
        wifiWasConnected = true;
    }

    return wifiWasConnected;

}

bool network_WiFi::WiFiConfig()
{
    wifiManager.startConfigPortal(AP_WIFI, AP_PASSWORD);

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiWasConnected = true;
        return true;
    }
    else
    {
        wifiWasConnected = false;
        return false;
    }
}

bool network_WiFi::disconnect()
{
    if (wifiWasConnected) // Hanya disconnect jika sebelumnya terhubung
    {
        Serial.println("Disconnecting from WiFi...");
        WiFi.disconnect(); // Memutuskan koneksi WiFi
        wifiWasConnected = false; // Mengubah status koneksi
        return true; // Mengembalikan true jika disconnect berhasil
    }
    else
    {
        Serial.println("WiFi was not connected.");
        return false; // Mengembalikan false jika tidak terhubung
    }
}