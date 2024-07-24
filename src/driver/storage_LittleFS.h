#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <interfaces/storage_interface.h>

class storage_LittleFS : public storage_interface
{

public:
    bool init();
    bool writeFile(std::string filename, std::string data);
    std::string readFile(std::string filename);
    bool deleteFile(std::string filename);
    bool exists(std::string filename);
};