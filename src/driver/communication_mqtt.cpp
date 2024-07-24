#include "communication_mqtt.h"

communication_mqtt::communication_mqtt(const char *server, const char *clientID, const char *user, const char *pass)
    : mqtt_server(server), mqtt_clientID(clientID), mqtt_user(user), mqtt_pass(pass), client(espClient) {}

bool communication_mqtt::init()
{
    client.setServer(mqtt_server, 1883);
    return true;
}

bool communication_mqtt::connect()
{
    if (mqtt_user != nullptr && mqtt_pass != nullptr)
    {
        return client.connect(mqtt_clientID, mqtt_user, mqtt_pass);
    }
    else
    {
        return client.connect(mqtt_clientID);
    }
}

bool communication_mqtt::publish(const char *topic, const char *payload)
{
    return client.publish(topic, payload);
}

bool communication_mqtt::subscribe(const char *topic)
{
    return client.subscribe(topic);
}

void communication_mqtt::loop()
{
    client.loop();
}

void communication_mqtt::setCallback(std::function<void(char *, uint8_t *, unsigned int)> callback)
{
    _callback = callback;
    client.setCallback([this](char *topic, uint8_t *payload, unsigned int length)
                            {
        if (_callback) {
            _callback(topic, payload, length);
        } });
}
