#pragma once

class communication_interface
{
public:
    virtual bool init() = 0;
    virtual bool connect() = 0;
    virtual bool publish(const char* topic, const char* payload) = 0;
    virtual bool subscribe(const char* topic) = 0;
    virtual bool connected() = 0;
    virtual bool disconnect() = 0;
    virtual void loop() = 0;
    virtual void setCallback(std::function<void(char*, uint8_t*, unsigned int)> callback) = 0;
};
