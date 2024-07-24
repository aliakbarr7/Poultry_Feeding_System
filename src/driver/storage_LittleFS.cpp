#include "storage_LittleFS.h"

bool storage_LittleFS::init()
{
    if (!LittleFS.begin(true))
    { // Mount the filesystem with format on fail option
        Serial.println("LittleFS mount failed, attempting to format...");
        if (!LittleFS.format())
        {
            Serial.println("Formatting LittleFS failed!");
            return false;
        }
        else
        {
            Serial.println("LittleFS formatted successfully.");
            if (!LittleFS.begin())
            {
                Serial.println("Failed to mount LittleFS after formatting!");
                return false;
            }
        }
    }
    Serial.println("LittleFS mounted successfully.");
    return true;
}

bool storage_LittleFS::writeFile(std::string filename, std::string data)
{
    File file = LittleFS.open(filename.c_str(), "w");
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return false;
    }
    if (file.print(data.c_str()))
    {
        file.close();
        return true;
    }
    else
    {
        Serial.println("Failed to write to file");
        file.close();
        return false;
    }
}

std::string storage_LittleFS::readFile(std::string filename)
{
    File file = LittleFS.open(filename.c_str(), "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return "";
    }
    std::string data = file.readString().c_str();
    file.close();
    return data;
}

bool storage_LittleFS::deleteFile(std::string filename)
{
    if (LittleFS.remove(filename.c_str()))
    {
        return true;
    }
    else
    {
        Serial.println("Failed to delete file");
        return false;
    }
}

bool storage_LittleFS::exists(std::string filename)
{
    return LittleFS.exists(filename.c_str());
}
