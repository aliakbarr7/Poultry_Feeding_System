#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <interfaces/communication_interface.h>

class communication_mqtt : public communication_interface
{
public:
    communication_mqtt(const char* server, const char* clientID, const char* user = nullptr, const char* pass = nullptr);
    bool init();
    bool connect();
    bool connected();
    bool disconnect();
    bool publish(const char* topic, const char* payload);
    bool subscribe(const char* topic);
    void loop();
    void setCallback(std::function<void(char*, uint8_t*, unsigned int)> callback);



private:
    const char* mqtt_server;
    const char* mqtt_clientID;
    const char* mqtt_user;
    const char* mqtt_pass;
    WiFiClient espClient;
    PubSubClient client;
    std::function<void(char*, uint8_t*, unsigned int)> _callback;

};
